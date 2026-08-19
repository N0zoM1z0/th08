#!/usr/bin/env python3
"""Check tracked Markdown links and canonical TH08 target facts.

This is a structural check, not a prose freshness oracle.  Use it in public CI
to catch renamed local files and accidental target-identity drift without
requiring the private executable.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
import subprocess
import tomllib
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
INLINE_LINK = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
REFERENCE_LINK = re.compile(r"^\s*\[[^\]]+\]:\s*(\S+)", re.MULTILINE)
SHA256 = re.compile(r"\b[0-9a-fA-F]{64}\b")


def tracked_markdown() -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "-z", "*.md"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [ROOT / entry.decode() for entry in result.stdout.split(b"\0") if entry]


def local_destination(raw: str) -> str | None:
    value = raw.strip()
    if value.startswith("<") and value.endswith(">"):
        value = value[1:-1]
    # An optional Markdown title follows a whitespace-delimited destination.
    value = value.split(maxsplit=1)[0]
    if not value or value.startswith(("#", "http://", "https://", "mailto:")):
        return None
    return unquote(value.split("#", 1)[0])


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog="Example:\n  python3 scripts/check-docs.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.parse_args()

    errors: list[str] = []
    checked_links = 0
    try:
        with (ROOT / "config" / "target.toml").open("rb") as stream:
            expected_sha = str(tomllib.load(stream)["target"]["sha256"]).lower()
        markdown = tracked_markdown()
        for path in markdown:
            text = path.read_text(encoding="utf-8")
            for raw in INLINE_LINK.findall(text) + REFERENCE_LINK.findall(text):
                destination = local_destination(raw)
                if destination is None:
                    continue
                checked_links += 1
                resolved = (path.parent / destination).resolve()
                if not resolved.is_relative_to(ROOT):
                    errors.append(
                        f"{path.relative_to(ROOT)}: local link escapes repository: {raw}"
                    )
                elif not resolved.exists():
                    errors.append(
                        f"{path.relative_to(ROOT)}: missing local link target: {destination}"
                    )
            for line_number, line in enumerate(text.splitlines(), start=1):
                if "th08.exe" not in line.lower():
                    continue
                for digest in SHA256.findall(line):
                    if digest.lower() != expected_sha:
                        errors.append(
                            f"{path.relative_to(ROOT)}:{line_number}: "
                            "th08.exe SHA-256 differs from config/target.toml"
                        )
    except (OSError, KeyError, subprocess.CalledProcessError, tomllib.TOMLDecodeError) as exc:
        errors.append(str(exc))

    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1
    print(
        f"documentation OK: {len(markdown)} tracked Markdown files, "
        f"{checked_links} local links"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
