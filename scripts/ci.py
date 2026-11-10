#!/usr/bin/env python3
"""Fast public checks for the whole checkout; no target or compiler required.

Requires Python 3.11+ and PyYAML. Run affected comparator/runtime tests
separately; --full explicitly selects the larger public test suite.
"""
from pathlib import Path
import argparse
import ast
import json
import subprocess
import sys
import tempfile
import tomllib

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True


def run(*arguments):
    subprocess.run(arguments, cwd=ROOT, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--full", action="store_true", help="also run the broad public suite")
    args = parser.parse_args()
    try:
        import yaml

        # Inspect the entire tree, including staged and new local inputs. A clean
        # or shallow PR checkout must not hide errors in earlier PR commits.
        names = subprocess.check_output(
            ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"], cwd=ROOT
        ).decode().split("\0")
        for name in sorted(set(names) - {""}):
            path = ROOT / name
            if name.startswith(("build/", ".analysis/", ".tools/", "scripts/prefix/", "scripts/dls/")):
                raise ValueError(f"private/generated path tracked: {name}")
            if path.suffix.lower() in {
                ".exe", ".dll", ".obj", ".o", ".lib", ".a", ".pch", ".pdb",
                ".idb", ".i64", ".gpr", ".dat", ".msi", ".key", ".pem", ".token",
            } or any(part.endswith(".rep") for part in path.parts):
                raise ValueError(f"private/build artifact tracked: {name}")
            if not path.is_file() or path.is_symlink():
                continue
            with path.open("rb") as stream:
                magic = stream.read(4)
            if magic.startswith((b"MZ", b"\x7fELF")):
                raise ValueError(f"executable tracked: {name}")
            if path.suffix == ".py":
                ast.parse(path.read_text(encoding="utf-8"), filename=name)
            elif path.suffix == ".json":
                json.loads(path.read_text(encoding="utf-8"))
            elif path.suffix == ".toml":
                tomllib.loads(path.read_text(encoding="utf-8"))
            elif path.suffix in {".yml", ".yaml"}:
                yaml.safe_load(path.read_text(encoding="utf-8"))
            elif path.suffix == ".sh" or name in {"scripts/wineth08", "scripts/create_th08_prefix"}:
                # bash -n accepts multiple arguments but checks only the first file.
                run("bash", "-n", name)
        print("Whole-tree syntax and private-artifact checks passed", flush=True)

        for script, options in (
            ("validate-tracking.py", ["--skip-target-bytes"]),
            ("check-match-config.py", []),
            ("validate-library.py", []),
        ):
            run(sys.executable, "-B", f"scripts/{script}", *options)

        # Exercise each Ninja configuration without changing the working build.
        import configure

        original_scripts_dir = configure.SCRIPTS_DIR
        try:
            with tempfile.TemporaryDirectory(prefix="th08-ci-config-") as directory:
                configure.SCRIPTS_DIR = Path(directory) / "scripts"
                for build_type in configure.BuildType:
                    configure.configure(build_type)
        finally:
            configure.SCRIPTS_DIR = original_scripts_dir
        run("git", "diff", "--check", "HEAD")
        if args.full:
            run(sys.executable, "scripts/ci-full.py")
    except (ImportError, OSError, ValueError, SyntaxError, subprocess.CalledProcessError) as exc:
        print(f"error: CI validation failed: {exc}", file=sys.stderr)
        return 1
    except yaml.YAMLError as exc:
        print(f"error: CI validation failed: {exc}", file=sys.stderr)
        return 1
    print("TH08 basic checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
