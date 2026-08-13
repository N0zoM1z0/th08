# IDA and analysis safety

## Current hard stop

The IDA MCP bridge is attached to the file open in the IDA GUI. The current GUI
database is TH07, not TH08. There is no program selector that safely redirects
a request to another IDB, so TH08 IDA analysis is presently **fail-closed**:

- do not use current MCP disassembly/decompilation as TH08 evidence;
- do not rename, retype, comment, or otherwise write TH08 findings into TH07;
- do not patch bytes with IDA, MCP, Ghidra, or another tool.

The absence of a TH08 IDA session is an analysis limitation, not permission to
weaken target verification or claim a match.

## Required IDA preflight

IDA MCP may be used for TH08 only after the exact executable is open and every
item below is confirmed from the active database and original file:

| Gate | Required value |
| --- | --- |
| Version | original Japanese 1.00d |
| SHA-256 | `330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924` |
| File size | `840704` bytes |
| Image base | `0x00400000` |
| Entry point | `0x004A619E` |
| `.text` range | `0x00402000`–`0x004B3B77` |

If any value is missing or differs, stop. Re-run this preflight for each new
IDA session and after any GUI file/database switch. IDA's function boundaries,
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
