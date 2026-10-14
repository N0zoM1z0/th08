#!/usr/bin/env python3
"""Show recorded replay-suite results and the latest capture progress."""
import argparse
from collections import Counter
import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def tail_lines(path, limit=65536):
    try:
        with path.open("rb") as stream:
            stream.seek(max(0, os.fstat(stream.fileno()).st_size - limit))
            return stream.read(limit).decode("utf-8", errors="replace").splitlines()
    except FileNotFoundError:
        return []


def progress(suite, case):
    logs = []
    for role in ("reference", "candidate"):
        for path in (suite / case / (role + ".log"), suite / case / role / "gdb.log"):
            try:
                logs.append((path.stat().st_mtime_ns, role, path))
            except FileNotFoundError:
                continue
    if not logs:
        return "capture starting"
    # Files created in the same filesystem timestamp tick can tie; prefer inner progress.
    _, role, log = max(logs, key=lambda entry: (entry[0], entry[2].name == "gdb.log"))
    lines = tail_lines(log)
    last = lines[-1] if lines else ""
    for line in reversed(lines):
        if line.startswith(("Captured ", "Portable replay:")):
            return f"{role}: {line}"
    try:
        completion = json.loads(last)
    except (ValueError, TypeError):
        capture = suite / case / role
        rows, metadata = capture / "rows.jsonl", capture / "metadata.json"
        if rows.exists() and metadata.exists():
            try:
                fields = json.loads(metadata.read_text()).get("schema", {}).get("fields", [])
            except (OSError, ValueError):
                return f"{role}: capture updating"
            for line in reversed(tail_lines(rows)):
                try:
                    row = json.loads(line)
                    values = dict(zip(fields, row)) if isinstance(row, list) else row
                    return (f"{role}: calculation {values['calc']}, "
                            f"stage index {values['stage']}, gameplay frame {values['frame']}")
                except (ValueError, TypeError, KeyError):
                    continue
        return f"{role}: see {log.relative_to(ROOT)}" if last else f"{role}: capture starting"
    if isinstance(completion, dict) and "rows" in completion:
        return f"{role}: captured {completion['rows']} calculation frames"
    return f"{role}: capture updating"


def live_games(suite, proc=Path("/proc")):
    """Select native runtimes by cwd and portable runtimes by their capture destination."""
    result = []
    if not proc.is_dir():
        return None
    for folder in proc.iterdir():
        if not folder.name.isdigit():
            continue
        try:
            name = (folder / "comm").read_text().strip()
            if name not in ("th08.exe", "th08-modern"):
                continue
            cwd = (folder / "cwd").resolve(strict=True)
            capture = None
            if not cwd.is_relative_to(suite):
                if name != "th08-modern" or not cwd.is_relative_to(ROOT / "build"):
                    continue
                # Read this one ownership field; never print the process environment.
                for entry in (folder / "environ").read_bytes().split(b"\0"):
                    if entry.startswith(b"TH08_PORTABLE_CAPTURE="):
                        capture = Path(os.fsdecode(entry.split(b"=", 1)[1])).resolve()
                        break
                if capture is None or not capture.is_relative_to(suite):
                    continue
            resident = int((folder / "statm").read_text().split()[1]) * os.sysconf("SC_PAGE_SIZE")
            result.append((int(folder.name), resident, (capture or cwd).relative_to(suite)))
        except (OSError, ValueError, IndexError):
            continue
    return sorted(result)


def artifact_bytes(folder):
    total = 0
    for parent, _, files in os.walk(folder, followlinks=False):
        for name in files:
            path = Path(parent) / name
            try:
                if not path.is_symlink():
                    total += path.stat().st_size
            except FileNotFoundError:
                pass
    return total


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--suite", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, default=ROOT / "config/replay-fixtures.json")
    parser.add_argument("--resources", action="store_true", help="Show live game RSS and retained artifact sizes")
    parser.add_argument("--wine-prefix", type=Path, help="Include this runner-owned shared Wine prefix in the resource report")
    args = parser.parse_args()
    suite = args.suite.resolve()
    if not suite.is_relative_to(ROOT / "build"):
        parser.error("--suite must be under build/")
    prefix = args.wine_prefix.resolve() if args.wine_prefix else None
    if prefix and (not prefix.is_relative_to(ROOT / "build") or not (prefix / ".th08-replay-owned").is_file()):
        parser.error("--wine-prefix must be a runner-owned prefix under build/")
    fixtures = {f["id"] for f in json.loads(args.manifest.read_text())["fixtures"]}
    cases = json.loads((suite / "suite.json").read_text())["cases"]
    if set(cases) - fixtures:
        parser.error("Suite contains cases absent from the manifest")
    counts = Counter(c["result"] for c in cases.values())
    calculations = sum(c["comparedFrames"] for c in cases.values() if c["result"] == "equal")
    print(f"Recorded results: {counts['equal']}/{len(fixtures)} equal; {counts['different']} different, "
          f"{counts['error']} errors, {counts['running']} running, {counts['fetched']} fetched")
    print(f"Compared calculations in equal cases: {calculations}")
    for case, result in cases.items():
        if result["result"] == "running":
            print(f"Running: {case}; {progress(suite, case)}")
        elif result["result"] in ("different", "error"):
            print(f"Review: {case}; {result['result']}; see {(suite / case).relative_to(ROOT)}")
    if args.resources:
        games = live_games(suite)
        if games is None:
            print("Live game process inspection: unavailable on this host")
        else:
            print(f"Live game processes: {len(games)}")
            for pid, resident, cwd in games:
                print(f"  PID {pid}: {resident / 1048576:.1f} MiB RSS; {cwd}")
        print(f"Logical suite file size: {artifact_bytes(suite) / 1048576:.1f} MiB "
              "(shared hardlinks included; archive symlinks excluded)")
        if prefix:
            print(f"Logical shared Wine prefix size: {artifact_bytes(prefix) / 1048576:.1f} MiB")


if __name__ == "__main__":
    raise SystemExit(main())
