"""Reviewed, bounded initialized-data contracts for COFF and linked PE32 images.

Contract sizes are observations, never inferred from neighboring target symbols.
Every pointer slot is typed and checked; there is no unresolved-pointer mask.
"""

from __future__ import annotations

from functools import lru_cache
import hashlib
from pathlib import Path
import re
import struct
import tomllib

from coff import ObjectModule
from pe_image import PEImage


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "config/data-contracts.toml"
MAP_PUBLIC = re.compile(
    r"^\s+[0-9A-Fa-f]{4}:[0-9A-Fa-f]{8}\s+(\S+)\s+([0-9A-Fa-f]{8})(?:\s|$)"
)


class DataError(ValueError):
    def __init__(self, code: str, message: str, **details: object):
        super().__init__(message)
        self.finding = {"code": code, "message": message, **details}


def stable_symbol(name: str) -> str:
    return re.sub(r"(?<=@)\?A0x[0-9a-fA-F]{8}(?=@)", "?A0x*", name)


def repository_path(value: str, root: Path = ROOT) -> Path:
    path = (root / value).resolve()
    if not path.is_relative_to(root.resolve()):
        raise DataError("CONTRACT_PATH_INVALID", f"path escapes repository: {value}")
    return path


def hex_bytes(value: object, field: str) -> bytes:
    try:
        if not isinstance(value, str):
            raise ValueError("expected a hexadecimal string")
        return bytes.fromhex(value)
    except ValueError as exc:
        raise DataError("CONTRACT_SCHEMA_INVALID", f"{field}: {exc}") from exc


def validate_manifest(
    document: dict, target_sha256: str, root: Path = ROOT
) -> list[dict]:
    if not isinstance(document, dict) or set(document) != {
        "schema_version",
        "target_sha256",
        "contracts",
    }:
        raise DataError(
            "CONTRACT_SCHEMA_INVALID",
            "expected schema_version, target_sha256 and contracts",
        )
    if (
        type(document["schema_version"]) is not int
        or document["schema_version"] != 1
        or document["target_sha256"] != target_sha256
    ):
        raise DataError(
            "CONTRACT_TARGET_INVALID",
            "contract schema or target hash differs from target.toml",
        )
    contracts = document["contracts"]
    if not isinstance(contracts, list) or not contracts:
        raise DataError("CONTRACT_SCHEMA_INVALID", "contracts must be a nonempty array")
    names, ranges = set(), []
    for contract in contracts:
        if not isinstance(contract, dict):
            raise DataError("CONTRACT_SCHEMA_INVALID", "each contract must be a table")
        required = {
            "name",
            "kind",
            "object",
            "symbol",
            "target_address",
            "size",
            "evidence",
        }
        kind = contract.get("kind")
        payload = {"data_hex"} if kind == "bytes" else {"entries"}
        if (
            kind not in {"bytes", "pointer_array"}
            or set(contract) != required | payload
        ):
            raise DataError(
                "CONTRACT_SCHEMA_INVALID",
                "unknown kind, missing field or unexpected contract field",
            )
        for key in ("name", "object", "symbol", "evidence"):
            if not isinstance(contract[key], str) or not contract[key].strip():
                raise DataError("CONTRACT_SCHEMA_INVALID", f"{key} must be nonempty")
        if contract["name"] in names:
            raise DataError(
                "CONTRACT_SCHEMA_INVALID", f"duplicate contract: {contract['name']}"
            )
        names.add(contract["name"])
        repository_path(contract["object"], root)
        if not contract["object"].startswith("build/") or not contract[
            "object"
        ].endswith(".obj"):
            raise DataError(
                "CONTRACT_SCHEMA_INVALID", "contract owner must be a build/ COFF object"
            )
        address, size = contract["target_address"], contract["size"]
        if (
            type(address) is not int
            or type(size) is not int
            or not 0 < size <= 0x100000
        ):
            raise DataError(
                "CONTRACT_SCHEMA_INVALID",
                "target_address/size must be integers with a bounded positive size",
            )
        if not 0 <= address < address + size <= 0x100000000:
            raise DataError(
                "CONTRACT_SCHEMA_INVALID", "contract extent is outside PE32 addresses"
            )
        for lo, hi in ranges:
            if address < hi and lo < address + size:
                raise DataError(
                    "CONTRACT_SCHEMA_INVALID",
                    "overlapping target contracts need separate reviewed ownership",
                )
        ranges.append((address, address + size))
        if kind == "bytes":
            if len(hex_bytes(contract["data_hex"], "data_hex")) != size:
                raise DataError(
                    "CONTRACT_SCHEMA_INVALID",
                    "data_hex length differs from explicit size",
                )
            continue
        entries = contract["entries"]
        if not isinstance(entries, list) or not entries or size != len(entries) * 4:
            raise DataError(
                "CONTRACT_SCHEMA_INVALID",
                "pointer-array size must cover every four-byte entry",
            )
        for entry in entries:
            if not isinstance(entry, dict):
                raise DataError(
                    "CONTRACT_SCHEMA_INVALID", "each pointer entry must be a table"
                )
            keys = {"kind"}
            if entry.get("kind") == "cstring":
                keys |= {"target", "data_hex", "symbol", "addend"}
                data = hex_bytes(entry.get("data_hex"), "entry.data_hex")
                if not data or data[-1] != 0 or b"\0" in data[:-1]:
                    raise DataError(
                        "CONTRACT_SCHEMA_INVALID",
                        "cstring must contain exactly one trailing NUL",
                    )
            elif entry.get("kind") == "symbol":
                keys |= {"target", "symbol", "addend"}
            elif entry.get("kind") != "null":
                raise DataError(
                    "CONTRACT_SCHEMA_INVALID",
                    "pointer kind must be cstring, symbol or null",
                )
            if entry["kind"] != "null":
                if not isinstance(entry.get("symbol"), str) or not entry["symbol"]:
                    raise DataError(
                        "CONTRACT_SCHEMA_INVALID",
                        "symbol pointer needs its decorated COFF name",
                    )
                if (
                    type(entry.get("addend")) is not int
                    or not 0 <= entry["addend"] <= 0xFFFFFFFF
                ):
                    raise DataError(
                        "CONTRACT_SCHEMA_INVALID",
                        "symbol addend must be an unsigned dword",
                    )
            if set(entry) != keys:
                raise DataError(
                    "CONTRACT_SCHEMA_INVALID",
                    "unexpected or missing pointer-entry fields",
                )
            if "target" in entry and (
                type(entry["target"]) is not int
                or not 0 < entry["target"] <= 0xFFFFFFFF
            ):
                raise DataError(
                    "CONTRACT_SCHEMA_INVALID",
                    "pointer target must be a nonzero PE32 address",
                )
    return sorted(contracts, key=lambda row: (row["target_address"], row["name"]))


@lru_cache(maxsize=4)
def _load_contracts(content: bytes, target_sha256: str) -> list[dict]:
    return validate_manifest(tomllib.loads(content.decode("utf-8")), target_sha256)


def load_contracts(path: Path = MANIFEST) -> list[dict]:
    with (ROOT / "config/target.toml").open("rb") as stream:
        target = tomllib.load(stream)["target"]
    return _load_contracts(path.read_bytes(), target["sha256"])


def attest_target(image: PEImage) -> None:
    with (ROOT / "config/target.toml").open("rb") as stream:
        expected = tomllib.load(stream)["target"]
    if image.sha256 != expected["sha256"] or len(image.data) != expected["size"]:
        raise DataError(
            "TARGET_IDENTITY_INVALID",
            "expected the canonical Japanese TH08 1.00d executable",
        )


def read_va(image: PEImage, address: int, size: int) -> bytes:
    return image.read_rva(address - image.image_base, size)


def target_bytes(contract: dict, target: PEImage) -> bytes:
    expected = read_va(target, contract["target_address"], contract["size"])
    if contract["kind"] == "bytes":
        if expected != bytes.fromhex(contract["data_hex"]):
            raise DataError(
                "CONTRACT_TARGET_MISMATCH",
                "reviewed data bytes differ from canonical target",
            )
    else:
        for index, entry in enumerate(contract["entries"]):
            pointer = struct.unpack_from("<I", expected, index * 4)[0]
            if pointer != entry.get("target", 0):
                raise DataError(
                    "CONTRACT_TARGET_MISMATCH",
                    "reviewed pointer differs from canonical target",
                    index=index,
                )
            if entry["kind"] == "cstring":
                data = bytes.fromhex(entry["data_hex"])
                if read_va(target, pointer, len(data)) != data:
                    raise DataError(
                        "CONTRACT_TARGET_MISMATCH",
                        "reviewed string differs from canonical target",
                        index=index,
                    )
    return expected


@lru_cache(maxsize=16)
def _load_object(content: bytes) -> ObjectModule:
    module = ObjectModule()
    module.unpack(content)
    return module


def load_object(path: Path) -> ObjectModule:
    # Cache parsed content, never a timestamp-based claim about mutable inputs.
    return _load_object(path.read_bytes())


def symbol_name(module: ObjectModule, symbol) -> str:
    return symbol.get_name(module.string_table).decode("ascii", errors="strict")


def owner(module: ObjectModule, name: str):
    found = [
        symbol
        for symbol in module.symbols
        if symbol.section_number > 0
        and stable_symbol(symbol_name(module, symbol)) == stable_symbol(name)
    ]
    if len(found) != 1:
        code = "DATA_OWNER_MISSING" if not found else "DATA_OWNER_AMBIGUOUS"
        raise DataError(code, f"expected one data owner for {name}, found {len(found)}")
    return found[0]


def source_bytes(module: ObjectModule, symbol, offset: int, size: int) -> bytes:
    if symbol.section_number <= 0:
        raise DataError(
            "DATA_SOURCE_UNAVAILABLE", "pointer destination has no COFF definition"
        )
    section = module.sections[symbol.section_number - 1]
    start = symbol.value + offset
    if section.flags & (0x20 | 0x80) or section.data is None:
        raise DataError(
            "DATA_SOURCE_UNAVAILABLE",
            "expected initialized data, not code or zero-fill storage",
        )
    if start < symbol.value or start + size > len(section.data):
        raise DataError(
            "DATA_OWNER_EXTENT",
            "explicit data extent exceeds its COFF storage",
            required_size=size,
        )
    # This is an object-side safety bound, not a way to invent the target size.
    for other in module.symbols:
        if (
            other.section_number == symbol.section_number
            and symbol.value < other.value < start + size
        ):
            if not symbol_name(module, other).startswith("."):
                raise DataError(
                    "DATA_OWNER_EXTENT",
                    "explicit data extent crosses a different COFF owner",
                )
    return bytes(section.data[start : start + size])


def literal_bytes(module: ObjectModule, symbol, addend: int) -> bytes | None:
    """Bound a compiler literal; string proof covers its first NUL terminator."""
    name = symbol_name(module, symbol)
    real = re.fullmatch(r"__real@([0-9a-fA-F]{8}|[0-9a-fA-F]{16})", name)
    if real:
        return source_bytes(module, symbol, addend, len(real.group(1)) // 2)
    if not name.startswith(("??_C@_0", "??_C@_1")):
        return None
    width = 2 if name.startswith("??_C@_1") else 1
    # Explicitly bound literal decoding; do not walk neighboring data owners.
    if symbol.section_number <= 0:
        raise DataError(
            "LITERAL_SOURCE_UNAVAILABLE", "compiler string has no COFF definition"
        )
    source_bytes(module, symbol, addend, 0)
    section = module.sections[symbol.section_number - 1]
    end = min(
        [
            other.value
            for other in module.symbols
            if other.section_number == symbol.section_number
            and other.value > symbol.value
        ]
        + [len(section.data or b"")]
    )
    start = symbol.value + addend
    for stop in range(start + width, min(end, start + 0x10000) + 1, width):
        if section.data[stop - width : stop] == bytes(width):
            return source_bytes(module, symbol, addend, stop - start)
    raise DataError(
        "LITERAL_UNTERMINATED", "compiler string has no bounded NUL terminator"
    )


def check_object(contract: dict, target: PEImage, module: ObjectModule) -> dict:
    expected = target_bytes(contract, target)
    symbol = owner(module, contract["symbol"])
    actual = source_bytes(module, symbol, 0, contract["size"])
    section = module.sections[symbol.section_number - 1]
    relocations = {}
    for relocation in section.relocations:
        offset = relocation.virtual_address - symbol.value
        if 0 <= offset < contract["size"]:
            if (
                offset in relocations
                or relocation.type != 6
                or offset + 4 > contract["size"]
            ):
                raise DataError(
                    "DATA_RELOCATION_INVALID",
                    "data needs unique, bounded DIR32 relocations",
                )
            relocations[offset] = relocation
    if contract["kind"] == "bytes":
        if relocations:
            raise DataError(
                "DATA_RELOCATION_INVALID",
                "a plain-byte contract contains pointer relocations",
            )
        compare_bytes(actual, expected)
    else:
        wanted = {
            index * 4
            for index, entry in enumerate(contract["entries"])
            if entry["kind"] != "null"
        }
        if set(relocations) != wanted:
            raise DataError(
                "DATA_RELOCATION_SET",
                "missing or extra pointer initializer",
                expected_offsets=sorted(wanted),
                actual_offsets=sorted(relocations),
            )
        for index, entry in enumerate(contract["entries"]):
            offset = index * 4
            addend = struct.unpack_from("<I", actual, offset)[0]
            if entry["kind"] == "null":
                if addend:
                    raise DataError(
                        "DATA_POINTER_MISMATCH", "null entry is nonzero", index=index
                    )
                continue
            referenced = module.symbols[relocations[offset].symbol_table_index]
            if (
                stable_symbol(symbol_name(module, referenced))
                != stable_symbol(entry["symbol"])
                or addend != entry["addend"]
            ):
                raise DataError(
                    "DATA_POINTER_IDENTITY",
                    "pointer symbol or addend differs",
                    index=index,
                    actual_symbol=symbol_name(module, referenced),
                    actual_addend=addend,
                )
            if entry["kind"] == "cstring":
                data = bytes.fromhex(entry["data_hex"])
                compare_bytes(
                    source_bytes(module, referenced, addend, len(data)),
                    data,
                    index=index,
                )
    return {
        "status": "verified",
        "bytes": contract["size"],
        "object_symbol": symbol_name(module, symbol),
    }


def compare_bytes(actual: bytes, expected: bytes, **details: object) -> None:
    if actual != expected:
        offset = next(
            (i for i, pair in enumerate(zip(actual, expected)) if pair[0] != pair[1]),
            min(len(actual), len(expected)),
        )
        raise DataError(
            "DATA_CONTENT_MISMATCH",
            "initialized data differs",
            offset=offset,
            expected_hex=expected[offset : offset + 16].hex(),
            actual_hex=actual[offset : offset + 16].hex(),
            **details,
        )


def parse_publics(path: Path) -> dict[str, set[int]]:
    result: dict[str, set[int]] = {}
    for line in path.read_text(encoding="cp1252", errors="strict").splitlines():
        match = MAP_PUBLIC.match(line)
        if match:
            result.setdefault(match.group(1), set()).add(int(match.group(2), 16))
    if not result:
        raise DataError("LINK_MAP_INVALID", "linker map contains no address symbols")
    return result


def linked_address(publics: dict[str, set[int]], name: str) -> int:
    addresses = {
        address
        for symbol, values in publics.items()
        if stable_symbol(symbol) == stable_symbol(name)
        for address in values
    }
    if len(addresses) != 1:
        raise DataError(
            "LINK_SYMBOL_UNRESOLVED",
            f"expected one linked address for {name}, found {len(addresses)}",
            symbol=name,
        )
    return next(iter(addresses))


def check_linked(
    contract: dict, target: PEImage, rebuild: PEImage, publics: dict[str, set[int]]
) -> dict:
    expected = target_bytes(contract, target)
    address = linked_address(publics, contract["symbol"])
    actual = read_va(rebuild, address, contract["size"])
    if contract["kind"] == "bytes":
        compare_bytes(actual, expected)
    else:
        for index, entry in enumerate(contract["entries"]):
            pointer = struct.unpack_from("<I", actual, index * 4)[0]
            if entry["kind"] == "cstring":
                data = bytes.fromhex(entry["data_hex"])
                try:
                    content = read_va(rebuild, pointer, len(data))
                except ValueError as exc:
                    raise DataError(
                        "DATA_POINTER_INVALID",
                        "linked string pointer has no raw data",
                        index=index,
                        pointer=f"0x{pointer:08X}",
                    ) from exc
                compare_bytes(content, data, index=index)
            wanted = (
                0
                if entry["kind"] == "null"
                else (linked_address(publics, entry["symbol"]) + entry["addend"])
                & 0xFFFFFFFF
            )
            if pointer != wanted:
                raise DataError(
                    "DATA_POINTER_IDENTITY",
                    "linked pointer selects a different owner",
                    index=index,
                    expected=f"0x{wanted:08X}",
                    actual=f"0x{pointer:08X}",
                )
    return {
        "status": "verified",
        "bytes": contract["size"],
        "linked_address": f"0x{address:08X}",
    }


def check(
    contract: dict,
    target: PEImage,
    *,
    object_path: Path | None = None,
    linked: tuple[PEImage, dict[str, set[int]]] | None = None,
) -> dict:
    if object_path is None and linked is None:
        raise DataError(
            "DATA_SCOPE_EMPTY",
            "choose COFF or linked verification; an unchecked contract cannot pass",
        )
    report = {
        "contract": contract["name"],
        "target_address": f"0x{contract['target_address']:08X}",
        "object": contract["object"],
        "size": contract["size"],
        "findings": [],
        "coff": {"status": "not_checked"},
        "linked": {"status": "not_checked"},
    }
    for scope in ("coff", "linked"):
        if (
            scope == "coff"
            and object_path is None
            or scope == "linked"
            and linked is None
        ):
            continue
        try:
            if scope == "coff":
                content = object_path.read_bytes()
                report["object_input"] = {
                    "path": str(object_path),
                    "size": len(content),
                    "sha256": hashlib.sha256(content).hexdigest(),
                }
                report[scope] = check_object(contract, target, _load_object(content))
            else:
                report[scope] = check_linked(contract, target, *linked)
        except (OSError, KeyError, IndexError, ValueError, struct.error) as exc:
            finding = (
                exc.finding
                if isinstance(exc, DataError)
                else {"code": "DATA_INPUT_INVALID", "message": str(exc)}
            )
            report[scope] = {"status": "mismatch"}
            report["findings"].append(
                {"scope": scope, "contract": contract["name"], **finding}
            )
    report["result"] = "mismatch" if report["findings"] else "verified"
    return report


def file_identity(path: Path) -> dict:
    data = path.read_bytes()
    return {
        "path": str(path),
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
    }
