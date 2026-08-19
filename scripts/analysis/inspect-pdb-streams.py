#!/usr/bin/env python3
"""List or safely extract streams from a Microsoft MSF 7.0 PDB file."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct


ROOT = Path(__file__).resolve().parents[2]
MSF7_MAGIC = b"Microsoft C/C++ MSF 7.00\r\n\x1aDS\x00\x00\x00"
NIL_STREAM = 0xFFFFFFFF


def pages_for_size(size: int, page_size: int) -> int:
    return (size + page_size - 1) // page_size


def page_bytes(data: bytes, page_size: int, page: int) -> bytes:
    start = page * page_size
    end = start + page_size
    if page < 0 or end > len(data):
        raise ValueError(f"page {page} is outside the PDB")
    return data[start:end]


def parse(path: Path) -> tuple[dict[str, int], list[dict[str, object]]]:
    data = path.read_bytes()
    if len(data) < 0x38 or data[: len(MSF7_MAGIC)] != MSF7_MAGIC:
        raise ValueError("input is not an MSF 7.0 PDB")
    page_size, free_page, page_count, directory_size, reserved, block_map = (
        struct.unpack_from("<6I", data, 0x20)
    )
    if page_size < 512 or page_size > 65536 or page_size & (page_size - 1):
        raise ValueError(f"invalid page size: {page_size}")
    if page_count * page_size > len(data) or page_count == 0:
        raise ValueError("PDB page count exceeds file size")
    directory_page_count = pages_for_size(directory_size, page_size)
    if directory_page_count * 4 > page_size:
        raise ValueError("multi-page directory block maps are not supported")
    block_map_data = page_bytes(data, page_size, block_map)
    directory_pages = list(
        struct.unpack_from(f"<{directory_page_count}I", block_map_data, 0)
    )
    directory = b"".join(
        page_bytes(data, page_size, page) for page in directory_pages
    )[:directory_size]
    if len(directory) < 4:
        raise ValueError("truncated PDB stream directory")
    stream_count = struct.unpack_from("<I", directory, 0)[0]
    sizes_end = 4 + stream_count * 4
    if sizes_end > len(directory):
        raise ValueError("truncated PDB stream-size table")
    sizes = list(struct.unpack_from(f"<{stream_count}I", directory, 4))
    cursor = sizes_end
    streams: list[dict[str, object]] = []
    for index, size in enumerate(sizes):
        if size == NIL_STREAM:
            streams.append({"index": index, "size": None, "pages": []})
            continue
        count = pages_for_size(size, page_size)
        end = cursor + count * 4
        if end > len(directory):
            raise ValueError(f"truncated page list for stream {index}")
        pages = list(struct.unpack_from(f"<{count}I", directory, cursor))
        cursor = end
        for page in pages:
            page_bytes(data, page_size, page)
        streams.append({"index": index, "size": size, "pages": pages})
    header = {
        "page_size": page_size,
        "free_page_map": free_page,
        "page_count": page_count,
        "directory_size": directory_size,
        "reserved": reserved,
        "block_map": block_map,
    }
    return header, streams


def extract_stream(path: Path, stream: dict[str, object], page_size: int) -> bytes:
    size = stream["size"]
    if size is None:
        raise ValueError(f"stream {stream['index']} is absent")
    data = path.read_bytes()
    payload = b"".join(
        page_bytes(data, page_size, int(page)) for page in stream["pages"]
    )
    return payload[: int(size)]


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "Examples:\n"
            "  python3 scripts/analysis/inspect-pdb-streams.py\n"
            "  python3 scripts/analysis/inspect-pdb-streams.py --extract 3\n"
            "  python3 scripts/analysis/inspect-pdb-streams.py other.pdb --json"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "pdb", nargs="?", type=Path, default=ROOT / "build" / "vc70.pdb"
    )
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--extract", action="append", type=int, default=[])
    parser.add_argument(
        "--output-dir", type=Path, default=ROOT / ".analysis" / "pdb-streams"
    )
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()
    path = args.pdb.expanduser().resolve()
    try:
        header, streams = parse(path)
        by_index = {int(stream["index"]): stream for stream in streams}
        missing = sorted(set(args.extract) - set(by_index))
        if missing:
            raise ValueError("unknown stream indexes: " + ", ".join(map(str, missing)))
        if args.extract:
            output_dir = args.output_dir.expanduser().resolve()
            output_dir.mkdir(parents=True, exist_ok=True)
            for index in args.extract:
                output = output_dir / f"stream-{index}.bin"
                if output.exists() and not args.force:
                    raise ValueError(f"refusing to overwrite {output}; pass --force")
                output.write_bytes(
                    extract_stream(path, by_index[index], int(header["page_size"]))
                )
                print(f"wrote {output}")
    except (OSError, KeyError, TypeError, ValueError, struct.error) as exc:
        print(f"error: PDB inspection: {exc}")
        return 1
    if args.json:
        print(json.dumps({"header": header, "streams": streams}, indent=2))
    else:
        print(
            f"{path}: page_size={header['page_size']} pages={header['page_count']} "
            f"directory={header['directory_size']} streams={len(streams)}"
        )
        for stream in streams:
            size = "absent" if stream["size"] is None else str(stream["size"])
            print(
                f"{int(stream['index']):4d}\t{size:>10}\t"
                + ",".join(str(page) for page in stream["pages"])
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
