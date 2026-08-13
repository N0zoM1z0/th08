# IDA and analysis safety

## Session boundary

The IDA MCP bridge is attached to the file open in the IDA GUI; it has no
program selector. Fail closed until the active database is established as TH08.
Never reinterpret a response from another game database as TH08, and never
patch bytes with IDA, MCP, Ghidra, or another tool.

IDA decompilation, names, types, and function extents are semantic/navigation
evidence only. They never establish an exact match; that remains a reproducible
comparison against the canonical `resources/th08.exe`.

## Required IDA preflight

IDA MCP may be used for TH08 only after the canonical file is verified and the
active database's mapped image passes one of the two attestation paths below.
The bridge may report an IDB/loader input hash rather than the canonical PE
file hash, so a different reported database hash is not automatically a
different mapped image.

| Gate | Required value |
| --- | --- |
| Version | original Japanese 1.00d |
| Canonical file SHA-256 | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| File size | `840704` bytes |
| Image base | `0x00400000` |
| Entry point | `0x004A619E` |
| `.text` range | `0x00402000`–`0x004B3B77` |

First run `python3 scripts/verify-target.py`. Then either confirm the active
database reports the canonical file SHA-256 and size, or, when its reported
hash is a database/loader artifact, compare read-only IDA bytes against the
canonical file at all of: `.text` start (`0x00402000`), entry
(`0x004A619E`), the function under study, and at least two separated `.text`
locations. The samples must agree byte-for-byte and the active image base,
entry point, and `.text` extent must match the table. Record the sample
addresses in the analysis handoff.

If either attestation path fails, stop. Re-run this preflight for each new IDA
session and after any GUI file/database switch. IDA's function boundaries,
types, and decompiler output remain hypotheses even after the gate passes.

## Read-only fallback

The target can be inspected without IDA:

```bash
mkdir -p .analysis
sha256sum resources/th08.exe
stat -c '%s' resources/th08.exe
objdump -x resources/th08.exe
objdump -d -Mintel resources/th08.exe > .analysis/th08-objdump.txt
```

Use `llvm-objdump` equivalents when GNU objdump cannot decode the PE cleanly.
Keep dumps under ignored `.analysis/`; do not commit the executable or dump.

## Ghidra

Import only the verified target into a dedicated local TH08 project, allow
analysis to finish, then re-check image base, entry point, and section extent.
The retained `scripts/export_ghidra_objs.py --import-csv` path first runs the
project's target verifier, then imports `resources/th08.exe` into a disposable
local project through Ghidra's `analyzeHeadless` runner. Existing databases and
remote `ghidra://` servers are refused because their input identity and mutable
state are not attested. Exported mappings and delinked objects remain proposals
and require target-side review before committing or comparison acceptance.
