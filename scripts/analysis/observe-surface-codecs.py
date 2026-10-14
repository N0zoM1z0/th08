#!/usr/bin/env python3
"""Read pinned-archive probe colors with Wine hardware breakpoints; never patch code."""
import argparse
import ctypes
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import signal
import struct
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("replay_ptrace", ROOT / "scripts/replay-ptrace.py")
trace = importlib.util.module_from_spec(spec)
spec.loader.exec_module(trace)
import sys
sys.path.insert(0, str(ROOT / "scripts"))
from library_match import pe_bytes_at


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--map", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--extended", action="store_true")
    args = parser.parse_args()
    if os.uname().machine != "x86_64":
        parser.error("Hardware observer requires Linux x86-64")
    text = args.map.read_text()
    symbols = {}
    for line in text.splitlines():
        match = re.match(r"\s+[0-9a-fA-F]+:[0-9a-fA-F]+\s+(\S+)\s+([0-9a-fA-F]{8})\s", line)
        if match:
            symbols[match[1]] = int(match[2], 16)
    names = ["_D3DXLoadSurfaceFromSurface@32",
             "?Encode@CD3DXCodec_A1R5G5B5@@UAEXIIPAUD3DXCOLOR@@@Z",
             "?Encode@CD3DXCodec_A8R8G8B8@@UAEXIIPAUD3DXCOLOR@@@Z",
             "?BltTriangle2D@CD3DXBlt@@IAEJXZ"]
    addresses = [symbols[name] for name in names]
    # Both encoders reach their pixel loop at +0x64, after conversion and FPU setup.
    addresses[1] += 0x64
    addresses[2] += 0x64
    # Canonical 0x00480DEA follows the source-row Decode call in this archive body.
    addresses[3] += 0x259
    executable = args.executable.read_bytes()
    attest = [(address, pe_bytes_at(executable, address, 8)) for address in addresses]
    observations = []
    captured_bytes = 0
    current_case = -1
    case_count = 640 if args.extended else 160
    child = subprocess.Popen(["wine", str(args.executable), *(["--extended"] if args.extended else [])],
                             preexec_fn=lambda: trace.ptrace(0, 0))
    inferior = trace.Inferior(child.pid)
    started = time.monotonic()
    installed = False
    def deadline(signum, frame):
        raise TimeoutError("Surface codec observation deadline exceeded")
    signal.signal(signal.SIGALRM, deadline)
    signal.setitimer(signal.ITIMER_REAL, 85)
    try:
        _, status = os.waitpid(child.pid, 0)
        if not os.WIFSTOPPED(status):
            raise RuntimeError("Wine did not stop at initial exec")
        trace.ptrace(0x4200, child.pid, 0, 0x10 | 0x100000)
        trace.ptrace(7, child.pid)
        while True:
            if time.monotonic() - started > 85:
                raise TimeoutError("Surface codec observation deadline exceeded")
            if installed:
                _, status = os.waitpid(child.pid, 0)
            else:
                time.sleep(.05)
                waited, status = os.waitpid(child.pid, os.WNOHANG)
                if not waited:
                    os.kill(child.pid, signal.SIGSTOP)
                    _, status = os.waitpid(child.pid, 0)
            if os.WIFEXITED(status) or os.WIFSIGNALED(status):
                child.returncode = os.waitstatus_to_exitcode(status)
                break
            stopped = os.WSTOPSIG(status)
            event = status >> 16
            if not installed:
                try:
                    mapped = all(inferior.read_memory(address, 8) == code for address, code in attest)
                except OSError:
                    mapped = False
                if mapped:
                    for index, address in enumerate(addresses):
                        trace.ptrace(6, child.pid, 848 + index * 8, address)
                    trace.ptrace(6, child.pid, 848 + 7 * 8, 0x55)
                    installed = True
            elif stopped == signal.SIGTRAP and not event:
                registers = trace.Registers()
                trace.ptrace(12, child.pid, 0, ctypes.addressof(registers))
                if registers.rip in addresses:
                    kind = addresses.index(registers.rip)
                    if kind == 0:
                        current_case += 1
                    else:
                        stack = struct.unpack("<4I", inferior.read_memory(registers.rsp & 0xffffffff, 16))
                        if kind == 3:
                            codec = struct.unpack("<I", inferior.read_memory(registers.rbx & 0xffffffff, 4))[0]
                            width = struct.unpack("<I", inferior.read_memory(codec + 0x1058, 4))[0]
                            buffer = registers.rdi & 0xffffffff
                            row = struct.unpack("<I", inferior.read_memory((registers.rbp & 0xffffffff) - 0x28, 4))[0]
                        else:
                            frame = registers.rbp & 0xffffffff
                            codec = struct.unpack("<I", inferior.read_memory(frame - 4, 4))[0]
                            width = struct.unpack("<I", inferior.read_memory(codec + 0x1058, 4))[0]
                            buffer = struct.unpack("<I", inferior.read_memory(frame + 0x10, 4))[0]
                            row = struct.unpack("<I", inferior.read_memory(frame + 8, 4))[0]
                        if not 0 < width <= 1024 or not 0 <= current_case < case_count:
                            raise ValueError("Observed codec bounds differ")
                        pixels = inferior.read_memory(buffer, width * 16)
                        captured_bytes += len(pixels)
                        if captured_bytes > 32 * 1024 * 1024:
                            raise ValueError("Surface codec observation exceeds 32 MiB")
                        fp = ctypes.create_string_buffer(512)
                        trace.ptrace(14, child.pid, 0, ctypes.addressof(fp))
                        observation = dict(case=current_case, kind="decode" if kind == 3 else "encode",
                                           row=row, width=width, colorBits=pixels.hex(),
                                           x87Control=struct.unpack_from("<H", fp.raw)[0])
                        if kind != 3:
                            observation["roundingBiasBits"] = inferior.read_memory(registers.rdi & 0xffffffff, 16).hex()
                        observations.append(observation)
                    trace.ptrace(6, child.pid, 848 + 6 * 8, 0)
                    registers.eflags |= 0x10000
                    trace.ptrace(13, child.pid, 0, ctypes.addressof(registers))
                    stopped = 0
            trace.ptrace(7, child.pid, 0, 0 if event or stopped == signal.SIGSTOP else stopped)
        if child.returncode != 0 or current_case != case_count - 1 or not observations:
            raise RuntimeError("Surface codec capture did not complete all expected cases")
        report = dict(evidenceClass="Read-only hardware observations inside the pinned-archive API probe",
                      executableSha256=hashlib.sha256(executable).hexdigest(),
                      observerSha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                      capturedColorBytes=captured_bytes,
                      mapSha256=hashlib.sha256(args.map.read_bytes()).hexdigest(), cases=current_case + 1,
                      attestedInstructions=[dict(address=hex(a), bytes=b.hex()) for a, b in attest],
                      observations=observations)
        args.output.write_text(json.dumps(report, indent=2) + "\n")
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)
        if child.returncode is None:
            os.kill(child.pid, signal.SIGKILL)
            os.waitpid(child.pid, 0)


if __name__ == "__main__":
    main()
