#!/usr/bin/env python3
"""Run muted replay windows with normal life rules and retain render evidence."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from replay_file import CORPUS, verify_fixture

ROOT = Path(__file__).resolve().parents[1]
WINDOWS = (
    ("border-transitions", "easy-0-b", 0, 90),
    ("yukari-stage3", "easy-5-b", 2, 90),
    ("border-stage5", "lunatic-0-b", 5, 90),
    ("magic-final-a", "lunatic-1-a", 6, 60),
    ("ghost-final-b", "lunatic-3-b", 7, 60),
    ("scarlet-extra", "extra-2-extra", 8, 60),
)


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--windows", help="Comma-separated window IDs; default: all six")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build"):
        parser.error("--output-dir must be under build/")
    windows = WINDOWS
    if args.windows:
        selected = set(args.windows.split(","))
        windows = [window for window in WINDOWS if window[0] in selected]
        if selected != {window[0] for window in windows}:
            parser.error("Unknown portable replay window")
    output.mkdir(parents=True, exist_ok=False)
    candidate = args.candidate.resolve(strict=True)
    data = args.game_data_dir.absolute()
    fixtures = {fixture["id"]: fixture for fixture in json.loads(
        (ROOT / "config/replay-fixtures.json").read_text())["fixtures"]}
    report = dict(candidateSha256=sha(candidate),
                  sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT).decode().strip(),
                  sourceDirty=bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
                  gameDataSha256=sha(data / "th08.dat"), bgmDataSha256=sha(data / "thbgm.dat"),
                  muted=True, keepLives=False, fastReplay=True, cosmeticEnemyNameCopySuppressed=False, windows=[])
    environment = dict(os.environ, TH08_RENDER_AUDIT_KEEP_LIVES="0", TH08_RENDER_AUDIT_FAST="1",
                       TH08_KEEP_RENDER_AUDIT_DIR="1", TH08_RENDER_AUDIT_SKIP_ENEMY_NAME="0")
    for index, (name, case, stage, seconds) in enumerate(windows, 1):
        fixture = fixtures[case]
        replay = CORPUS / "accepted" / fixture["file"]
        verify_fixture(replay, fixture)
        if stage not in [entry["index"] for entry in fixture["stages"]]:
            raise ValueError(f"Replay window stage is absent: {name}")
        print(f"[{index}/{len(windows)}] {name}: {case}, stage index {stage}, {seconds}s", flush=True)
        directory = output / name
        directory.mkdir()
        existing = set((ROOT / "build").glob("render-audit.*"))
        command = [str(ROOT / "scripts/audit-render-replay-linux.sh"), str(candidate),
                   str(data), str(replay), str(stage), str(seconds)]
        result = subprocess.run(command, cwd=ROOT, env=environment)
        created = set((ROOT / "build").glob("render-audit.*")) - existing
        if len(created) != 1:
            raise RuntimeError(f"Expected one owned audit runtime: {name}")
        runtime = created.pop()
        if runtime.is_symlink() or not runtime.resolve().is_relative_to(ROOT / "build"):
            raise RuntimeError("Audit runtime ownership differs")
        try:
            for source, destination in (("modern-enemy-render.csv", "enemy-render.csv.gz"),
                                        ("modern-files.txt", "files.txt.gz"),
                                        ("audit-runtime.log", "runtime.log.gz"),
                                        ("modern-crash.txt", "crash.txt.gz")):
                path = runtime / source
                if path.exists():
                    with path.open("rb") as stream, gzip.open(directory / destination, "wb") as target:
                        shutil.copyfileobj(stream, target)
        finally:
            shutil.rmtree(runtime)
        report["windows"].append(dict(id=name, fixture=case, replaySha256=fixture["sha256"],
                                      stage=stage, seconds=seconds,
                                      result="passed" if result.returncode == 0 else "failed"))
        (output / "report.json").write_text(json.dumps(report, indent=2, ensure_ascii=True) + "\n")
        if result.returncode:
            raise subprocess.CalledProcessError(result.returncode, command)
    print(f"Portable replay checks passed: {len(windows)} windows with enemy-render evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
