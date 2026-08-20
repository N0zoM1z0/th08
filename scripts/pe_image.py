"""Small fail-closed PE32 reader used by whole-image reconstruction tooling."""

from __future__ import annotations

from dataclasses import dataclass
import hashlib
from pathlib import Path
import struct


DIRECTORY_NAMES = (
    "export",
    "import",
    "resource",
    "exception",
    "security",
    "base_relocation",
    "debug",
    "architecture",
    "global_pointer",
    "tls",
    "load_config",
    "bound_import",
    "iat",
    "delay_import",
    "clr",
    "reserved",
)

RESOURCE_TYPE_NAMES = {
    1: "CURSOR",
    2: "BITMAP",
    3: "ICON",
    4: "MENU",
    5: "DIALOG",
    6: "STRING",
    7: "FONTDIR",
    8: "FONT",
    9: "ACCELERATOR",
    10: "RCDATA",
    11: "MESSAGETABLE",
    12: "GROUP_CURSOR",
    14: "GROUP_ICON",
    16: "VERSION",
    24: "MANIFEST",
}


class PEFormatError(ValueError):
    """Raised when an input cannot be parsed safely as the supported PE32 form."""


@dataclass(frozen=True)
class Section:
    name: str
    virtual_size: int
    rva: int
    raw_size: int
    raw_offset: int
    characteristics: int


class PEImage:
    """Parse the PE32 fields needed for deterministic reconstruction reports."""

    def __init__(self, path: Path | str):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        self.sha256 = hashlib.sha256(self.data).hexdigest()
        self._parse_headers()

    def _need(self, offset: int, size: int, what: str) -> None:
        if offset < 0 or size < 0 or offset + size > len(self.data):
            raise PEFormatError(f"{self.path}: truncated {what}")

    def _parse_headers(self) -> None:
        self._need(0, 0x40, "DOS header")
        if self.data[:2] != b"MZ":
            raise PEFormatError(f"{self.path}: missing MZ signature")
        self.pe_offset = struct.unpack_from("<I", self.data, 0x3C)[0]
        self._need(self.pe_offset, 24, "PE/COFF header")
        if self.data[self.pe_offset : self.pe_offset + 4] != b"PE\0\0":
            raise PEFormatError(f"{self.path}: missing PE signature")

        coff = self.pe_offset + 4
        (
            self.machine,
            self.section_count,
            self.timestamp,
            self.symbol_table_offset,
            self.symbol_count,
            self.optional_size,
            self.characteristics,
        ) = struct.unpack_from("<HHIIIHH", self.data, coff)

        optional = coff + 20
        self._need(optional, self.optional_size, "optional header")
        if self.optional_size < 96:
            raise PEFormatError(f"{self.path}: PE32 optional header is too small")
        self.magic = struct.unpack_from("<H", self.data, optional)[0]
        if self.magic != 0x10B:
            raise PEFormatError(f"{self.path}: only PE32 images are supported")

        self.linker_major, self.linker_minor = struct.unpack_from(
            "<BB", self.data, optional + 2
        )
        (
            self.size_of_code,
            self.size_of_initialized_data,
            self.size_of_uninitialized_data,
            self.entry_rva,
            self.base_of_code,
            self.base_of_data,
            self.image_base,
            self.section_alignment,
            self.file_alignment,
        ) = struct.unpack_from("<9I", self.data, optional + 4)
        (
            self.os_major,
            self.os_minor,
            self.image_major,
            self.image_minor,
            self.subsystem_major,
            self.subsystem_minor,
        ) = struct.unpack_from("<6H", self.data, optional + 40)
        (
            self.win32_version,
            self.size_of_image,
            self.size_of_headers,
            self.checksum,
        ) = struct.unpack_from("<4I", self.data, optional + 52)
        self.subsystem, self.dll_characteristics = struct.unpack_from(
            "<HH", self.data, optional + 68
        )
        (
            self.stack_reserve,
            self.stack_commit,
            self.heap_reserve,
            self.heap_commit,
            self.loader_flags,
            self.directory_count,
        ) = struct.unpack_from("<6I", self.data, optional + 72)

        directory_capacity = max(0, (self.optional_size - 96) // 8)
        parsed_directory_count = min(self.directory_count, directory_capacity)
        self.directories: list[dict[str, int | str]] = []
        for index in range(parsed_directory_count):
            rva, size = struct.unpack_from("<II", self.data, optional + 96 + index * 8)
            name = DIRECTORY_NAMES[index] if index < len(DIRECTORY_NAMES) else str(index)
            self.directories.append({"index": index, "name": name, "rva": rva, "size": size})

        section_table = optional + self.optional_size
        self._need(section_table, self.section_count * 40, "section table")
        self.sections: list[Section] = []
        for index in range(self.section_count):
            offset = section_table + index * 40
            raw_name = struct.unpack_from("<8s", self.data, offset)[0]
            name = raw_name.split(b"\0", 1)[0].decode("ascii", errors="replace")
            virtual_size, rva, raw_size, raw_offset = struct.unpack_from(
                "<IIII", self.data, offset + 8
            )
            characteristics = struct.unpack_from("<I", self.data, offset + 36)[0]
            if raw_size:
                self._need(raw_offset, raw_size, f"{name} raw data")
            self.sections.append(
                Section(name, virtual_size, rva, raw_size, raw_offset, characteristics)
            )

    def directory(self, index: int) -> tuple[int, int]:
        if index >= len(self.directories):
            return (0, 0)
        entry = self.directories[index]
        return int(entry["rva"]), int(entry["size"])

    def rva_to_offset(self, rva: int, size: int = 1) -> int:
        if rva < 0 or size < 0:
            raise PEFormatError(f"{self.path}: negative RVA or size")
        if rva < self.size_of_headers and rva + size <= min(self.size_of_headers, len(self.data)):
            return rva
        for section in self.sections:
            relative = rva - section.rva
            if 0 <= relative and relative + size <= section.raw_size:
                offset = section.raw_offset + relative
                self._need(offset, size, f"RVA {rva:#x}")
                return offset
        raise PEFormatError(f"{self.path}: RVA {rva:#x}+{size:#x} has no raw backing")

    def read_rva(self, rva: int, size: int) -> bytes:
        offset = self.rva_to_offset(rva, size)
        return self.data[offset : offset + size]

    def c_string_at_rva(self, rva: int, limit: int = 4096) -> str:
        offset = self.rva_to_offset(rva)
        end_limit = min(len(self.data), offset + limit)
        end = self.data.find(b"\0", offset, end_limit)
        if end < 0:
            raise PEFormatError(f"{self.path}: unterminated string at RVA {rva:#x}")
        return self.data[offset:end].decode("ascii", errors="replace")

    def raw_section_bytes(self, section: Section) -> bytes:
        return self.data[section.raw_offset : section.raw_offset + section.raw_size]

    def imports(self) -> list[dict[str, object]]:
        rva, size = self.directory(1)
        if not rva or not size:
            return []
        table_offset = self.rva_to_offset(rva, min(size, 20))
        imports: list[dict[str, object]] = []
        for descriptor_index in range(size // 20):
            offset = table_offset + descriptor_index * 20
            self._need(offset, 20, "import descriptor")
            original_thunk, timestamp, forwarder, name_rva, first_thunk = struct.unpack_from(
                "<5I", self.data, offset
            )
            if not any((original_thunk, timestamp, forwarder, name_rva, first_thunk)):
                break
            if not name_rva:
                raise PEFormatError(f"{self.path}: import descriptor lacks DLL name")
            dll = self.c_string_at_rva(name_rva)
            thunk_rva = original_thunk or first_thunk
            symbols: list[dict[str, int | str]] = []
            for thunk_index in range(65536):
                thunk = struct.unpack("<I", self.read_rva(thunk_rva + thunk_index * 4, 4))[0]
                if thunk == 0:
                    break
                if thunk & 0x80000000:
                    symbols.append({"kind": "ordinal", "ordinal": thunk & 0xFFFF})
                else:
                    hint = struct.unpack("<H", self.read_rva(thunk, 2))[0]
                    symbols.append(
                        {"kind": "name", "name": self.c_string_at_rva(thunk + 2), "hint": hint}
                    )
            else:
                raise PEFormatError(f"{self.path}: unterminated thunk table for {dll}")
            imports.append(
                {
                    "dll": dll,
                    "timestamp": timestamp,
                    "forwarder_chain": forwarder,
                    "symbols": symbols,
                }
            )
        return imports

    def _resource_name(self, root_offset: int, value: int) -> str:
        if not value & 0x80000000:
            return f"#{value & 0xFFFF}"
        relative = value & 0x7FFFFFFF
        offset = root_offset + relative
        self._need(offset, 2, "resource name length")
        length = struct.unpack_from("<H", self.data, offset)[0]
        self._need(offset + 2, length * 2, "resource name")
        return self.data[offset + 2 : offset + 2 + length * 2].decode(
            "utf-16le", errors="replace"
        )

    def resources(self) -> list[dict[str, object]]:
        root_rva, size = self.directory(2)
        if not root_rva or not size:
            return []
        root_offset = self.rva_to_offset(root_rva, min(size, 16))
        leaves: list[dict[str, object]] = []
        active: set[int] = set()

        def walk(relative: int, path: list[str], depth: int) -> None:
            if depth > 8:
                raise PEFormatError(f"{self.path}: resource tree exceeds depth limit")
            if relative in active:
                raise PEFormatError(f"{self.path}: cyclic resource directory")
            active.add(relative)
            directory_offset = root_offset + relative
            self._need(directory_offset, 16, "resource directory")
            named, ids = struct.unpack_from("<HH", self.data, directory_offset + 12)
            entry_count = named + ids
            self._need(directory_offset + 16, entry_count * 8, "resource directory entries")
            for index in range(entry_count):
                name_value, child_value = struct.unpack_from(
                    "<II", self.data, directory_offset + 16 + index * 8
                )
                component = self._resource_name(root_offset, name_value)
                child_relative = child_value & 0x7FFFFFFF
                child_path = [*path, component]
                if child_value & 0x80000000:
                    walk(child_relative, child_path, depth + 1)
                    continue
                data_entry = root_offset + child_relative
                self._need(data_entry, 16, "resource data entry")
                data_rva, data_size, codepage, reserved = struct.unpack_from(
                    "<4I", self.data, data_entry
                )
                payload = self.read_rva(data_rva, data_size)
                display_path = list(child_path)
                if display_path and display_path[0].startswith("#"):
                    type_id = int(display_path[0][1:])
                    type_name = RESOURCE_TYPE_NAMES.get(type_id)
                    if type_name:
                        display_path[0] = f"{type_name}({display_path[0]})"
                leaves.append(
                    {
                        "path": "/".join(display_path),
                        "raw_path": "/".join(child_path),
                        "size": data_size,
                        "codepage": codepage,
                        "reserved": reserved,
                        "sha256": hashlib.sha256(payload).hexdigest(),
                    }
                )
            active.remove(relative)

        walk(0, [], 0)
        return leaves

    def debug_entries(self) -> list[dict[str, object]]:
        rva, size = self.directory(6)
        if not rva or not size:
            return []
        offset = self.rva_to_offset(rva, size)
        entries: list[dict[str, object]] = []
        for index in range(size // 28):
            entry_offset = offset + index * 28
            (
                characteristics,
                timestamp,
                major,
                minor,
                debug_type,
                data_size,
                data_rva,
                data_offset,
            ) = struct.unpack_from("<IIHHIIII", self.data, entry_offset)
            payload = b""
            if data_size:
                if data_offset and data_offset + data_size <= len(self.data):
                    payload = self.data[data_offset : data_offset + data_size]
                elif data_rva:
                    payload = self.read_rva(data_rva, data_size)
                else:
                    raise PEFormatError(f"{self.path}: debug entry lacks data location")
            codeview = None
            if debug_type == 2 and payload:
                if payload.startswith(b"NB10") and len(payload) >= 16:
                    codeview = payload[16:].split(b"\0", 1)[0].decode("ascii", errors="replace")
                elif payload.startswith(b"RSDS") and len(payload) >= 24:
                    codeview = payload[24:].split(b"\0", 1)[0].decode("ascii", errors="replace")
            entries.append(
                {
                    "characteristics": characteristics,
                    "timestamp": timestamp,
                    "major": major,
                    "minor": minor,
                    "type": debug_type,
                    "size": data_size,
                    "data_rva": data_rva,
                    "data_offset": data_offset,
                    "sha256": hashlib.sha256(payload).hexdigest() if payload else None,
                    "codeview_path": codeview,
                }
            )
        return entries

    def metadata(self) -> dict[str, object]:
        return {
            "path": self.path.as_posix(),
            "size": len(self.data),
            "sha256": self.sha256,
            "machine": self.machine,
            "section_count": self.section_count,
            "timestamp": self.timestamp,
            "characteristics": self.characteristics,
            "linker_version": f"{self.linker_major}.{self.linker_minor}",
            "size_of_code": self.size_of_code,
            "size_of_initialized_data": self.size_of_initialized_data,
            "size_of_uninitialized_data": self.size_of_uninitialized_data,
            "entry_rva": self.entry_rva,
            "entry_address": self.image_base + self.entry_rva,
            "base_of_code": self.base_of_code,
            "base_of_data": self.base_of_data,
            "image_base": self.image_base,
            "section_alignment": self.section_alignment,
            "file_alignment": self.file_alignment,
            "os_version": f"{self.os_major}.{self.os_minor}",
            "image_version": f"{self.image_major}.{self.image_minor}",
            "subsystem_version": f"{self.subsystem_major}.{self.subsystem_minor}",
            "size_of_image": self.size_of_image,
            "size_of_headers": self.size_of_headers,
            "checksum": self.checksum,
            "subsystem": self.subsystem,
            "dll_characteristics": self.dll_characteristics,
            "stack_reserve": self.stack_reserve,
            "stack_commit": self.stack_commit,
            "heap_reserve": self.heap_reserve,
            "heap_commit": self.heap_commit,
            "loader_flags": self.loader_flags,
        }
