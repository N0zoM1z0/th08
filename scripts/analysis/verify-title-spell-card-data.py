#!/usr/bin/env python3
"""Compare reconstructed Title spell-card tables with Japanese TH08 1.00d."""

from __future__ import annotations

import argparse
import re
import struct
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from pe_image import PEImage  # noqa: E402


TARGET_SHA256 = "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924"
COMMENT_FORMATS_VA = 0x004C82C8
DIFFICULTY_NAMES_VA = 0x004C8628


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--target", type=Path, default=ROOT / "resources" / "th08.exe"
    )
    parser.add_argument(
        "--source",
        type=Path,
        default=ROOT / "src" / "TitleSpellCardData.inl",
    )
    return parser.parse_args()


def cpp_string_bytes(text: str) -> bytes:
    result = bytearray()
    cursor = 0
    while cursor < len(text):
        if text.startswith("\\x", cursor):
            result.append(int(text[cursor + 2 : cursor + 4], 16))
            cursor += 4
        else:
            result.append(ord(text[cursor]))
            cursor += 1
    return bytes(result)


def target_bytes(image: PEImage, va: int, size: int) -> bytes:
    return image.read_rva(va - image.image_base, size)


def target_c_string(image: PEImage, va: int) -> bytes:
    offset = image.rva_to_offset(va - image.image_base)
    end = image.data.find(b"\0", offset, offset + 0x400)
    if end < 0:
        raise ValueError(f"unterminated target string at {va:#010x}")
    return image.data[offset:end]


def parse_records(source: str) -> list[tuple[bytes, list[int], bytes, list[int]]]:
    records: list[tuple[bytes, list[int], bytes, list[int]]] = []
    for line in source.splitlines():
        if not line.startswith('    {"'):
            continue
        strings = re.findall(r'"((?:\\x[0-9A-Fa-f]{2}|[^"\\])*)"', line)
        arrays = re.findall(r"\{([0-9, ]+)\}", line)
        if len(strings) == 2 and len(arrays) == 2:
            records.append(
                (
                    cpp_string_bytes(strings[0]),
                    [int(value) for value in arrays[0].split(",")],
                    cpp_string_bytes(strings[1]),
                    [int(value) for value in arrays[1].split(",")],
                )
            )
    if len(records) != 18:
        raise ValueError(f"expected 18 Last Word records, found {len(records)}")
    return records


def parse_difficulty_names(source: str) -> list[bytes]:
    match = re.search(
        r"g_TitleSpellDifficultyNames\) = \{\n(.*?)\n\};", source, re.S
    )
    if match is None:
        raise ValueError("g_TitleSpellDifficultyNames initializer not found")
    names = [
        cpp_string_bytes(value)
        for value in re.findall(
            r'"((?:\\x[0-9A-Fa-f]{2}|[^"\\])*)"', match.group(1)
        )
    ]
    if len(names) != 6:
        raise ValueError(f"expected 6 difficulty names, found {len(names)}")
    return names


def verify_records(
    image: PEImage, records: list[tuple[bytes, list[int], bytes, list[int]]]
) -> None:
    for index, (format1, args1, format2, args2) in enumerate(records):
        record_va = COMMENT_FORMATS_VA + index * 0x30
        words = struct.unpack("<12I", target_bytes(image, record_va, 0x30))
        target_format1 = words[0]
        target_args1 = list(words[1:6])
        target_format2 = words[6]
        target_args2 = list(words[7:12])
        if target_c_string(image, target_format1) != format1:
            raise ValueError(f"Last Word record {index} line 1 differs")
        if target_args1 != args1:
            raise ValueError(f"Last Word record {index} line 1 args differ")
        if target_c_string(image, target_format2) != format2:
            raise ValueError(f"Last Word record {index} line 2 differs")
        if target_args2 != args2:
            raise ValueError(f"Last Word record {index} line 2 args differ")


def verify_difficulty_names(image: PEImage, names: list[bytes]) -> None:
    for index, name in enumerate(names):
        pointer = struct.unpack(
            "<I", target_bytes(image, DIFFICULTY_NAMES_VA + index * 4, 4)
        )[0]
        if target_c_string(image, pointer) != name:
            raise ValueError(f"spell-card difficulty name {index} differs")


def main() -> int:
    args = parse_args()
    try:
        image = PEImage(args.target)
        if image.sha256 != TARGET_SHA256:
            raise ValueError(
                f"unsupported target SHA-256 {image.sha256}; expected {TARGET_SHA256}"
            )
        source = args.source.read_text(encoding="utf-8")
        records = parse_records(source)
        names = parse_difficulty_names(source)
        verify_records(image, records)
        verify_difficulty_names(image, names)
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"title spell-card data verification failed: {exc}", file=sys.stderr)
        return 1

    print("title spell-card data: 18/18 records and 6/6 difficulty strings exact")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
