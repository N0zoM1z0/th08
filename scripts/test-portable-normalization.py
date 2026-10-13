#!/usr/bin/env python3
"""Probe the portable x86 D3DX normalization contracts without game assets."""
import argparse
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True, help="Debug x86 or x86_64 ELF")
    parser.add_argument("--expect-failure", action="store_true", help="Require the historical camera-particle rounding failure")
    args = parser.parse_args()
    candidate = args.candidate.resolve(strict=True)
    header = candidate.read_bytes()[:20]
    if header[:4] != b"\x7fELF" or header[5] != 1 or struct.unpack_from("<H", header, 18)[0] not in (3, 62):
        parser.error("The retail SSE normalization sample requires an x86 or x86_64 ELF")
    (ROOT / "build").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="portable-normalization.", dir=ROOT / "build") as name:
        temporary = Path(name)
        commands = temporary / "normalization.gdb"
        commands.write_text("""set pagination off
set confirm off
set debuginfod enabled off
start
python
import gdb, struct
inferior = gdb.selected_inferior()
source = int(gdb.parse_and_eval('&th08::g_Player.position'))
destination = int(gdb.parse_and_eval('&th08::g_Background.stagePosition'))
def normalize(values, alias=False):
    inferior.write_memory(source, struct.pack('<3I', *values))
    output = source if alias else destination
    result = int(gdb.parse_and_eval('D3DXVec3Normalize((D3DXVECTOR3*)%d, (D3DXVECTOR3*)%d)' % (output, source)))
    if result != output:
        raise RuntimeError('D3DXVec3Normalize returned the wrong output pointer')
    return struct.unpack('<3I', bytes(inferior.read_memory(output, 12)))
sample = (0xc15436fb, 0x43676bb8, 0x42bccfe0)
expected = (0xbd590edc, 0x3f6cb3cf, 0x3ec11f05)
actual = normalize(sample)
if actual != expected:
    print('FAIL: camera-particle normalization: expected %s, observed %s' % (expected, actual))
    gdb.execute('quit 1')
if normalize(sample, alias=True) != expected:
    raise RuntimeError('In-place normalization differs from separate output')
if normalize((0x33800000, 0, 0)) != (0, 0, 0):
    raise RuntimeError('A vector below the retail epsilon must normalize to zero')
boundary = normalize((0x34000000, 0, 0))
if not 0.99999 < struct.unpack('<f', struct.pack('<I', boundary[0]))[0] < 1.00001 or boundary[1:] != (0, 0):
    raise RuntimeError('A vector at the retail epsilon must remain nonzero')
for expression in ('D3DXVec3Normalize((D3DXVECTOR3*)0, (D3DXVECTOR3*)%d)' % source,
                   'D3DXVec3Normalize((D3DXVECTOR3*)%d, (D3DXVECTOR3*)0)' % destination):
    if int(gdb.parse_and_eval(expression)) != 0:
        raise RuntimeError('Null arguments must return null')
print('PASS: retail camera-particle bits, aliasing, epsilon boundary, and null arguments')
end
quit 0
""")
        environment = {key: value for key, value in os.environ.items() if not key.startswith("TH08_REPLAY_TRACE_")}
        environment.update(LC_ALL="C", SDL_AUDIODRIVER="dummy")
        result = subprocess.run(["gdb", "--batch", "--quiet", "-x", str(commands), "--args", str(candidate),
                                 "--data-dir", str(temporary)], cwd=temporary, env=environment,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
        messages = [line for line in result.stdout.splitlines() if line.startswith(("PASS:", "FAIL:"))]
        print("\n".join(messages), flush=True)
        if args.expect_failure:
            if result.returncode != 1 or "FAIL: camera-particle normalization:" not in result.stdout:
                raise RuntimeError("Historical normalization failure was not reproduced:\n" + result.stdout)
            print("Historical normalization failure reproduced", flush=True)
            return 0
        if not messages or result.returncode:
            raise RuntimeError("Normalization contracts failed:\n" + result.stdout)
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
