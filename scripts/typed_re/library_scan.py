#!/usr/bin/env python3
"""Fail-closed boundary for the not-yet-attested TH08 VC7 library scanner."""

from __future__ import annotations

import argparse
import json
import sys


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library")
    parser.add_argument("--min-size", type=int, default=8)
    parser.add_argument("--include-tracked", action="store_true")
    parser.add_argument("--address")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--json", action="store_true")
    parser.parse_args()
    print(
        json.dumps(
            {
                "result": "unsupported",
                "failure": {
                    "message": (
                        "TH08 VC7 library scanning is disabled until SHA-pinned archives, "
                        "a TH08 relocation allowlist, and comparator replay are configured"
                    )
                },
            },
            indent=2,
        ),
        file=sys.stderr,
    )
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
