#!/usr/bin/env nix-shell
#!nix-shell -p python311 -i python3

import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import tomllib

import ghidra_helpers

SCRIPT_PATH = Path(os.path.realpath(__file__)).parent
ROOT = SCRIPT_PATH.parent


def load_target_filename():
    with (ROOT / "config" / "target.toml").open("rb") as stream:
        return str(tomllib.load(stream)["target"]["filename"])


def verify_target(path):
    subprocess.run(
        [sys.executable, str(SCRIPT_PATH / "verify-target.py"), str(path)],
        check=True,
    )


def main():
    parser = argparse.ArgumentParser(
        description="Export TH08 original objects from a verified local executable",
    )
    parser.add_argument(
        "--import-csv",
        action="store_true",
        help="import the verified resources/th08.exe and apply mapping.csv",
    )
    args = parser.parse_args()
    if not args.import_csv:
        parser.error(
            "--import-csv is required; existing Ghidra databases are not attested"
        )

    target_filename = load_target_filename()
    target = ROOT / "resources" / target_filename
    verify_target(target)
    output_dir = ROOT / "build" / "objdiff" / "orig"
    os.makedirs(str(output_dir), exist_ok=True)
    with tempfile.TemporaryDirectory() as tempdir:
        mapping_csv = SCRIPT_PATH.parent / "config" / "mapping.csv"
        ghidra_helpers.runAnalyze(
            str(tempdir),
            "TH08",
            import_file=str(target),
            analysis=True,
            post_scripts=[
                ["ImportFromCsv.java", str(mapping_csv)],
                [
                    "ExportDelinker.java",
                    str(SCRIPT_PATH.parent / "config" / "ghidra_ns_to_obj.csv"),
                    str(output_dir),
                ],
            ],
        )


if __name__ == "__main__":
    main()
