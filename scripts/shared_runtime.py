#!/usr/bin/env python3
"""Repository-local forwarding entrypoint; Factory MCP is not required."""

import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def command(tool, *arguments, prefix=None, ghidra=None):
    runtime = ROOT / ".tools/factory-runtime.py"
    if not runtime.is_file():
        raise RuntimeError("Factory shared tools are not provisioned; see docs/SHARED_TOOLS.md")
    result = [sys.executable, str(runtime), "--repository", str(ROOT)]
    if prefix is not None:
        result += ["--prefix", str(prefix)]
    if ghidra is not None:
        result += ["--ghidra", str(ghidra)]
    return [*result, tool, "--", *(str(value) for value in arguments)]


if __name__ == "__main__":
    runtime = ROOT / ".tools/factory-runtime.py"
    if not runtime.is_file():
        raise SystemExit("Factory shared tools are not provisioned; see docs/SHARED_TOOLS.md")
    os.execv(sys.executable, [sys.executable, str(runtime), "--repository", str(ROOT), *sys.argv[1:]])
