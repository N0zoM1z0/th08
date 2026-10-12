#!/usr/bin/env python3
"""Compare a muted portable replay with a validated complete retail capture."""
import argparse
import gzip
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import struct
import subprocess
import tempfile

from replay_file import inspect as inspect_replay

ROOT = Path(__file__).resolve().parents[1]


def module(name):
    spec = importlib.util.spec_from_file_location(name.replace("-", "_"), ROOT / "scripts" / (name + ".py"))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def validate_reference(reference, replay, data, target):
    comparison, native = module("compare-replay-traces"), module("capture-replay")
    metadata, rows = comparison.load(reference)
    expected = dict(product="retail", executableSha256=sha(target), targetSha256=sha(target),
                    schema=json.loads((ROOT / "scripts/replay-external-schema.json").read_text()),
                    fixture=replay["sha256"], replay=replay, startStage=replay["stages"][0]["index"], playbackMode=2,
                    gameDataSha256=sha(data / "th08.dat"), bgmDataSha256=sha(data / "thbgm.dat"),
                    configSha256=hashlib.sha256(native.muted_config()).hexdigest(), muted=True)
    for field, value in expected.items():
        if metadata.get(field) != value:
            raise ValueError("Retail reference identity differs: " + field)
    fixture = next(entry for entry in json.loads((ROOT / "config/replay-fixtures.json").read_text())["fixtures"]
                   if entry["sha256"] == replay["sha256"])
    fingerprint = comparison.trace_sha256(metadata, rows)
    if len(rows) != fixture["retailTrace"]["calculationFrames"] or fingerprint != fixture["retailTrace"]["sha256"]:
        raise ValueError("Retail reference differs from the published trace expectation")
    return metadata, fixture, fingerprint


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--target", type=Path, default=ROOT / "resources/th08.exe")
    parser.add_argument("--replay", type=Path, required=True)
    parser.add_argument("--game-data-dir", type=Path, required=True)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=1800)
    parser.add_argument("--paced", action="store_true", help="Use the ordinary portable clock and rasterization")
    parser.add_argument("--backend", choices=("gdb", "recorder"), default="gdb",
                        help="Recorder requires a build with TH08_ENABLE_REPLAY_TRACE=ON")
    parser.add_argument("--max-calculations", type=int, help="Diagnostic prefix; cannot produce a complete parity result")
    parser.add_argument("--watch-score-stage", type=int, help="GDB: internal stage index for a score-call window")
    parser.add_argument("--watch-score-frame", type=int, help="GDB: first input frame for a score-call window")
    parser.add_argument("--start-stage", type=int, help="Start a diagnostic prefix at this recorded stage")
    parser.add_argument("--watch-item", type=int, help="GDB: record this item pool slot")
    parser.add_argument("--watch-item-start", type=int, default=1, help="GDB: first input frame for item motion records")
    parser.add_argument("--watch-enemy", type=int, help="GDB: record this enemy pool slot")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build") or output.exists():
        parser.error("--output-dir must be a fresh directory under build/")
    if args.timeout < 1 or (args.max_calculations is not None and args.max_calculations < 1):
        parser.error("Timeout and diagnostic calculation limit must be positive")
    if (args.watch_score_stage is None) != (args.watch_score_frame is None) or (args.watch_score_stage is not None and args.backend != "gdb"):
        parser.error("Score-call windows require both --watch-score-stage and --watch-score-frame with --backend gdb")
    if args.start_stage is not None and (args.backend != "gdb" or not args.max_calculations):
        parser.error("--start-stage requires a GDB diagnostic prefix")
    if args.watch_item is not None and (args.backend != "gdb" or not 0 <= args.watch_item <= 2096 or args.watch_item_start < 1):
        parser.error("Item motion records require --backend gdb, a slot from 0 to 2096, and a positive start frame")
    if args.watch_enemy is not None and (args.backend != "gdb" or not 0 <= args.watch_enemy < 480):
        parser.error("Enemy motion records require --backend gdb and a slot from 0 to 479")
    target, candidate, data, reference = [path.resolve(strict=True) for path in
                                         (args.target, args.candidate, args.game_data_dir, args.reference)]
    if target.stat().st_size != 840704 or sha(target) != "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924":
        parser.error("Target must be the canonical Japanese TH08 1.00d executable")
    comparison, native = module("compare-replay-traces"), module("capture-replay")
    replay = inspect_replay(args.replay)
    config = native.muted_config()
    metadata, fixture, fingerprint = validate_reference(reference, replay, data, target)
    start_stage = args.start_stage if args.start_stage is not None else metadata["startStage"]
    if start_stage not in [entry["index"] for entry in replay["stages"]]:
        parser.error("Selected diagnostic stage is absent from the recording")
    observer = ROOT / "scripts/gdb/portable-replay-observer.py"
    output.mkdir(parents=True)
    retained = dict(metadata, product="portable-linux", executableSha256=sha(candidate),
                    sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                    sourceTreeDirty=bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT)),
                    clockRate=1, rasterization=args.paced,
                    fastPortableClock=not args.paced, referenceTraceSha256=fingerprint,
                    observer=dict(type="GDB typed portable state reads" if args.backend == "gdb" else "compiled typed portable state reads",
                                  sha256=sha(observer) if args.backend == "gdb" else sha(ROOT / "src/modern/linux/replay_trace.cpp")),
                    backend=args.backend,
                    startStage=start_stage,
                    runnerSha256=sha(Path(__file__)),
                    muteMethod="SDL dummy audio driver; startup configuration muted",
                    environment=dict(host=platform.platform(), gdb=subprocess.check_output(["gdb", "--version"], text=True).splitlines()[0]))
    (output / "metadata.json").write_text(json.dumps(retained, indent=2) + "\n")
    settings = dict(reference=str(reference), replay=replay, playbackMode=metadata["playbackMode"],
                    maxCalculations=args.max_calculations, watchScoreStage=args.watch_score_stage, watchScoreFrame=args.watch_score_frame,
                    startStage=start_stage, watchItem=args.watch_item, watchItemStart=args.watch_item_start, watchEnemy=args.watch_enemy)
    (output / "settings.json").write_text(json.dumps(settings, indent=2) + "\n")
    runtime = Path(tempfile.mkdtemp(prefix="portable-state.", dir=ROOT / "build"))
    try:
        for archive in ("th08.dat", "thbgm.dat"):
            (runtime / archive).symlink_to(data / archive)
        (runtime / "th08.cfg").write_bytes(config)
        (runtime / "replay").mkdir()
        shutil.copyfile(args.replay, runtime / "replay/audit.rpy")
        environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1", GALLIUM_DRIVER="llvmpipe",
                           TH08_AUTOPLAY_REPLAY="./replay/audit.rpy", TH08_AUTOPLAY_STAGE=str(start_stage),
                           TH08_RENDER_AUDIT_FAST="0" if args.paced else "1", TH08_RENDER_AUDIT="0",
                           TH08_PORTABLE_CAPTURE=str(output), LP_NUM_THREADS="0", LP_NO_RAST="false" if args.paced else "true")
        environment["TH08_PORTABLE_REPOSITORY"] = str(ROOT)
        for name in ("TH08_REPLAY_TRACE_REFERENCE", "TH08_REPLAY_TRACE_ROWS", "TH08_REPLAY_TRACE_COMPLETION", "TH08_REPLAY_TRACE_MAXIMUM"):
            environment.pop(name, None)
        executable = [str(candidate), "--data-dir", str(runtime)]
        if args.backend == "gdb":
            gdb_commands = output / "capture.gdb"
            gdb_commands.write_text("set pagination off\nset confirm off\nset debuginfod enabled off\n"
                                    "set print thread-events off\nstart\nsource " + str(observer) + "\ncontinue\nquit\n")
            executable = ["gdb", "--batch", "--quiet", "-x", str(gdb_commands), "--args", *executable]
        else:
            binary_rows = struct.Struct("<25I")
            with (runtime / "reference.bin").open("wb") as destination, gzip.open(reference / "rows.jsonl.gz", "rt") as source:
                for line in source:
                    destination.write(binary_rows.pack(*json.loads(line)))
            environment.update(TH08_REPLAY_TRACE_REFERENCE=str(runtime / "reference.bin"),
                               TH08_REPLAY_TRACE_ROWS=str(runtime / "rows.bin"),
                               TH08_REPLAY_TRACE_COMPLETION=str(output / "complete.json"),
                               TH08_REPLAY_TRACE_MAXIMUM=str(args.max_calculations or 0))
        command = ["xvfb-run", "-a", "-s", "-screen 0 1024x768x24", "timeout", "--signal=TERM", "--kill-after=5s",
                   str(args.timeout) + "s", *executable]
        with (output / "gdb.log").open("w") as log:
            result = subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT)
        retained["runnerExitCode"] = result.returncode
        if args.backend == "recorder" and (runtime / "rows.bin").exists():
            previous = []
            with (runtime / "rows.bin").open("rb") as source, gzip.open(output / "rows.jsonl.gz", "wt") as destination, \
                    gzip.open(reference / "rows.jsonl.gz", "rt") as expected_stream:
                while raw := source.read(binary_rows.size):
                    row = list(binary_rows.unpack(raw))
                    destination.write(json.dumps(row) + "\n")
                    line = expected_stream.readline()
                    expected = json.loads(line) if line else []
                    if row != expected and not (output / "first-difference.json").exists():
                        fields = retained["schema"]["fields"]
                        difference = dict(calc=row[0], stage=row[1], frame=row[2],
                            differingFields=([field for field, a, b in zip(fields, expected, row) if a != b]
                                             if expected else ["traceLength"]),
                            reference=dict(zip(fields, expected)), candidate=dict(zip(fields, row)), previous=previous)
                        (output / "first-difference.json").write_text(json.dumps(difference, indent=2) + "\n")
                    previous = (previous + [dict(zip(retained["schema"]["fields"], row))])[-2:]
        complete_path = output / "complete.json"
        if result.returncode or not complete_path.exists():
            raise RuntimeError("Portable observer failed; inspect gdb.log.gz")
        complete = json.loads(complete_path.read_text())
        if complete["errors"]:
            raise RuntimeError("; ".join(complete["errors"]))
        if args.max_calculations:
            if not complete["diagnosticComplete"] or complete["rows"] != args.max_calculations:
                raise RuntimeError("Diagnostic prefix ended unexpectedly")
            print(f"Equal diagnostic prefix: {complete['rows']} calculations", flush=True)
        else:
            report = comparison.compare(reference, output, allow_pacing_difference=True)
            comparison.verify_expectation(report, fixture)
            (output / "comparison.json").write_text(json.dumps(report, indent=2) + "\n")
            print(f"{report['result']}: {report['comparedFrames']} calculations", flush=True)
    finally:
        for name in ("modern-crash.txt", "modern-files.txt"):
            path = runtime / name
            if path.exists():
                with path.open("rb") as source, gzip.open(output / (name + ".gz"), "wb") as destination:
                    shutil.copyfileobj(source, destination)
        shutil.rmtree(runtime)
        retained["temporaryRuntimeRemoved"] = True
        (output / "metadata.json").write_text(json.dumps(retained, indent=2) + "\n")
        log_path = output / "gdb.log"
        if log_path.exists():
            with log_path.open("rb") as source, gzip.open(str(log_path) + ".gz", "wb") as destination:
                shutil.copyfileobj(source, destination)
            log_path.unlink()


if __name__ == "__main__":
    main()
