#!/usr/bin/env python3
"""Check changed machine inputs; exact comparisons are selected by affected scope."""
from pathlib import Path
import ast
import json
import os
import subprocess
import sys
import tomllib

ROOT = Path(__file__).resolve().parents[1]


def main():
    if sys.argv[1:] in (["--help"], ["-h"]):
        print("usage: ci.py [--full | existing explicit check options]\nDefault: changed machine-input syntax and diff checks. Run affected exact/tool commands for the task.")
        return 0
    if len(sys.argv) > 1:
        arguments = sys.argv[1:]
        if not any(flag in arguments for flag in ('--full',)):
            print("Unknown CI option; use no arguments for scoped checks or --full for the broad suite.", file=sys.stderr)
            return 2
        arguments = [arg for arg in arguments if arg != "--full"]
        return subprocess.call([sys.executable, str(ROOT / "scripts/ci-full.py"), *arguments], cwd=ROOT)
    names = set()
    for command in (["git", "diff", "HEAD", "--name-only", "-z"],
                    ["git", "ls-files", "--others", "--exclude-standard", "-z"]):
        names.update(n for n in subprocess.check_output(command, cwd=ROOT).decode().split("\0") if n)
    if os.environ.get("CI"):
        # Clean CI checkouts have no worktree diff. Inspect the commit being checked.
        names.update(n for n in subprocess.check_output(
            ["git", "show", "--format=", "--first-parent", "--name-only", "-z", "HEAD"],
            cwd=ROOT).decode().split("\0") if n)
    for name in sorted(names):
        path = ROOT / name
        if not path.is_file() or path.is_symlink():
            continue
        if path.suffix == ".py":
            ast.parse(path.read_text(), filename=name)
        elif path.suffix == ".json":
            json.loads(path.read_text())
        elif path.suffix == ".toml":
            tomllib.loads(path.read_text())
        elif path.suffix == ".sh":
            subprocess.run(["bash", "-n", str(path)], check=True)
    subprocess.run(["git", "diff", "--check"], cwd=ROOT, check=True)
    print("Changed machine inputs checked. Use affected exact/tool checks; --full explicitly selects the broad suite.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
