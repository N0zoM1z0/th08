#!/usr/bin/env python3
"""Linux x86-64 tracer for a Wine PE32 external replay.

Uses hardware execute breakpoints and process_vm_readv; the shared observer
reads game memory and sends menu keys. Executable bytes are never written.
"""
import argparse
import ctypes
import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time
import types

libc = ctypes.CDLL(None, use_errno=True)
libc.ptrace.restype = ctypes.c_long
libc.ptrace.argtypes = (ctypes.c_uint, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p)
libc.process_vm_readv.restype = ctypes.c_ssize_t


def ptrace(request, pid, address=0, data=0):
    ctypes.set_errno(0)
    result = libc.ptrace(request, pid, ctypes.c_void_p(address), ctypes.c_void_p(data))
    if result == -1 and ctypes.get_errno():
        raise OSError(ctypes.get_errno(), f"ptrace request {request}")
    return result


class IOVec(ctypes.Structure):
    _fields_ = [("base", ctypes.c_void_p), ("length", ctypes.c_size_t)]


class Registers(ctypes.Structure):
    _fields_ = [(name, ctypes.c_ulonglong) for name in (
        "r15", "r14", "r13", "r12", "rbp", "rbx", "r11", "r10", "r9", "r8", "rax", "rcx", "rdx",
        "rsi", "rdi", "orig_rax", "rip", "cs", "eflags", "rsp", "ss", "fs_base", "gs_base", "ds", "es", "fs", "gs")]


class Inferior:
    def __init__(self, pid):
        self.pid = pid

    def read_memory(self, address, size):
        buffer = ctypes.create_string_buffer(size)
        local = IOVec(ctypes.addressof(buffer), size)
        remote = IOVec(address, size)
        copied = libc.process_vm_readv(self.pid, ctypes.byref(local), 1, ctypes.byref(remote), 1, 0)
        if copied != size:
            raise OSError(ctypes.get_errno(), f"Cannot read {size} bytes at 0x{address:x}")
        return buffer.raw


BREAKPOINTS = []


class Breakpoint:
    def __init__(self, location, kind):
        self.address = int(location[1:], 16)
        self.enabled = True
        if len(BREAKPOINTS) == 4:
            raise ValueError("x86 provides four hardware breakpoint slots")
        BREAKPOINTS.append(self)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine", required=True)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--timeout", type=float, required=True)
    parser.add_argument("--clock-rate", type=int, default=1)
    parser.add_argument("--clock-library", type=Path)
    args = parser.parse_args()
    if os.uname().machine != "x86_64":
        parser.error("The ptrace register layout requires Linux x86-64")
    output = Path.cwd().parent
    settings = json.loads((output / "observer.json").read_text())
    environment = os.environ.copy()
    gate = output / "clock-ready"
    if args.clock_library:
        environment.update(LD_PRELOAD=str(args.clock_library), TH08_REPLAY_CLOCK_RATE=str(args.clock_rate),
                           TH08_REPLAY_CLOCK_GATE=str(gate))

    def trace_child():
        ptrace(0, 0)  # PTRACE_TRACEME before exec; the tracer is this parent.

    child = subprocess.Popen([args.wine, str(args.executable)], env=environment, preexec_fn=trace_child,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    inferior = Inferior(child.pid)
    start = time.monotonic()
    deadline = start + args.timeout + 60
    observer_module = None
    installed = False
    last_registers = None
    last_progress = start

    def timer(signum, frame):
        if time.monotonic() >= deadline:
            raise TimeoutError("Replay capture deadline exceeded")

    signal.signal(signal.SIGALRM, timer)
    signal.setitimer(signal.ITIMER_REAL, 1, 1)
    try:
        _, status = os.waitpid(child.pid, 0)
        if not os.WIFSTOPPED(status):
            raise RuntimeError("Wine did not stop at its initial exec")
        ptrace(0x4200, child.pid, 0, 0x10 | 0x100000)  # TRACEEXEC | EXITKILL
        ptrace(7, child.pid)
        while True:
            if installed:
                _, status = os.waitpid(child.pid, 0)
            else:
                time.sleep(.1)
                waited, status = os.waitpid(child.pid, os.WNOHANG)
                if not waited:
                    os.kill(child.pid, signal.SIGSTOP)
                    _, status = os.waitpid(child.pid, 0)
            if os.WIFEXITED(status) or os.WIFSIGNALED(status):
                child.returncode = os.waitstatus_to_exitcode(status)
                raise RuntimeError(f"Wine exited before replay completion: {child.returncode}")
            stopped_signal = os.WSTOPSIG(status)
            event = status >> 16
            if not installed:
                try:
                    expected = bytes.fromhex(settings["attestBytes"])
                    mapped = inferior.read_memory(settings["attestAddress"], len(expected)) == expected
                except OSError:
                    mapped = False
                if mapped:
                    shim = types.ModuleType("gdb")
                    shim.Breakpoint = Breakpoint
                    shim.BP_HARDWARE_BREAKPOINT = 1
                    shim.selected_inferior = lambda: inferior
                    sys.modules["gdb"] = shim
                    spec = importlib.util.spec_from_file_location("observer", Path(__file__).with_name("replay-external-observer.py"))
                    observer_module = importlib.util.module_from_spec(spec)
                    spec.loader.exec_module(observer_module)
                    installed = True
                    deadline = time.monotonic() + args.timeout
                    gate.touch()
                elif time.monotonic() - start > 60:
                    raise TimeoutError("Wine did not map the attested target instructions")
            elif stopped_signal == signal.SIGTRAP and not event:
                debug_status = ptrace(3, child.pid, 848 + 6 * 8)  # PEEKUSER DR6
                registers = Registers()
                ptrace(12, child.pid, 0, ctypes.addressof(registers))
                hits = [bp for index, bp in enumerate(BREAKPOINTS)
                        if bp.enabled and debug_status & (1 << index) and registers.rip == bp.address]
                if hits:
                    for breakpoint in hits:
                        breakpoint.stop()
                    ptrace(6, child.pid, 848 + 6 * 8, 0)  # Clear DR6 status.
                    # Resume the trapped instruction once, as a hardware debugger does.
                    if not registers.eflags & 0x10000:
                        registers.eflags |= 0x10000
                        ptrace(13, child.pid, 0, ctypes.addressof(registers))
                    stopped_signal = 0
            if installed:
                configuration = tuple(bp.address if bp.enabled else 0 for bp in BREAKPOINTS)
                if configuration != last_registers:
                    for index, address in enumerate(configuration):
                        ptrace(6, child.pid, 848 + index * 8, address)
                    control = sum(1 << (index * 2) for index, address in enumerate(configuration) if address)
                    ptrace(6, child.pid, 848 + 7 * 8, control)
                    last_registers = configuration
                complete = output / "complete.json"
                if complete.exists():
                    report = json.loads(complete.read_text())
                    print(json.dumps(report), flush=True)
                    return 0 if report.get("ended") and not report.get("errors") else 1
                now = time.monotonic()
                if now - last_progress >= 10:
                    print(f"Captured {observer_module.observer.count} calculation frames", flush=True)
                    last_progress = now
            # Exec and our SIGSTOP are debugger events; Wine's other signals pass through.
            ptrace(7, child.pid, 0, 0 if event or stopped_signal == signal.SIGSTOP else stopped_signal)
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)
        if child.returncode is None:
            os.kill(child.pid, signal.SIGKILL)
            try:
                os.waitpid(child.pid, 0)
            except ChildProcessError:
                pass


if __name__ == "__main__":
    raise SystemExit(main())
