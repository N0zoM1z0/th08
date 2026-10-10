# Tool selection and command recipes

Use this page to choose an entry point. Public scripts are named by outcome;
build-internal generators are invoked by Ninja and normally should not be run
by hand.

For a first read, use the [documentation index](README.md) or
[project guide](PROJECT_GUIDE.md). The session checklist below applies when
starting writable reconstruction work; the question table routes individual
read-only investigations as well.

## Start every writable session

```bash
git status --short
python3 scripts/verify-target.py
python3 scripts/analysis/report-reconstruction-status.py --summary
python3 scripts/validate-tracking.py --require-target
```

Then read the [current handoff](RE_HANDOFF.md) and select one bounded target
from live ledger state. Search the
[knowledge map](KNOWLEDGE_BASE.md#existing-subject-index) before repeating
target analysis or compiler-shape probes.

## Choose the command by question

| Question | Command | Result and limit |
| --- | --- | --- |
| Is this the target executable? | `python3 scripts/verify-target.py` | Verifies file size and hashes. |
| What authored/library work remains? | `python3 scripts/analysis/report-reconstruction-status.py --summary` | Ledger-derived totals. Omit `--summary` for the default authored non-exact rows. |
| Which source locations need semantic review? | `python3 scripts/analysis/report-semantic-debt.py` | Candidate locations for raw offsets, absolute views, anonymous names, and opaque storage. Review each hit against its representation; byte-oriented code can be appropriate. |
| Which library rows lack mapping extents? | `python3 scripts/analysis/report-reconstruction-status.py --category library --state missing-size --sort address` | Live selector for extent-repair work. |
| Is library provenance/config structurally valid? | `python3 scripts/validate-library.py` | Public target-independent schema/ledger validation. Add `--require-archives` locally to hash-check configured private archives. |
| Is one configured library/archive unit exact? | `python3 scripts/compare-library.py UNIT --json` | Local target-required archive hash + member identity + COFF relocation replay + full `compare_size` target comparison. |
| Which pinned-archive library rows are review candidates? | `python3 scripts/analysis/propose-library-units.py --archive vc7-libcmt --min-size 0x20` | Read-only proposals with equal non-relocation bytes and COFF aux extents. Review and accept through `compare-library.py`. |
| Is library progress current? | `python3 scripts/library-progress.py --check` | Checks the separate library view in `docs/LIBRARY_PROGRESS.md` against accepted body bytes. |
| What does the target prove about one ABI/body? | `python3 scripts/typed-re.py ADDRESS --compare --json > build/typed-re-ADDRESS.json` | Read-only instruction/ABI facts; comparison is exact only if the configured report says so. |
| How do I build the normal executable? | `python3 scripts/build.py` | Regenerates `build.ninja` and links `build/th08.exe`. Add `--fresh` to clean generated Ninja outputs and known VC7/linker side outputs first. |
| How do I reproduce native Windows i386 validation? | Follow [WINDOWS_I386_RUNTIME.md](WINDOWS_I386_RUNTIME.md) in order. | Bootstraps VC7/DX8, cold-replays accepted normal units, verifies both final links, builds bugfix last, and deploys an unpatched isolated runtime. |
| What differs in the complete linked executable? | `python3 scripts/compare-whole-image.py --json > build/whole-image-report.json` | Verifies the canonical target, then reports PE headers/directories, section bytes, imports, resources, debug data, and accepted-unit linker-map address summaries. Add `--include-anchor-details` only when every per-function anchor is needed; it is diagnostic and never edits an exact ledger. |
| Which production object may combine target TUs? | `python3 scripts/analysis/report-tu-partition-candidates.py build/whole-image-anchors.json` | Ranks candidates by target-order inversions and drift jumps. Add `--object NAME.obj` for anchors, then inspect target neighbors to establish boundaries. |
| Which objects/archive members reference rebuild-only imports? | `python3 scripts/analysis/report-import-provenance.py` | Hash-checks inputs and locates references in COFF symbols and the linker map. Use it to select a target-membership investigation. |
| How do I build one configured object? | Read its `object` in `config/match-units.toml`, then use `python3 scripts/build.py TARGET` or the documented objdiff command. | Produces the object for subsequent comparison. |
| Is one configured function exact? | `python3 scripts/compare-function.py UNIT --json` | Canonical per-unit target comparison. |
| Did a shared change preserve one object's accepted units? | `python3 scripts/analysis/verify-exact-units.py --object build/probes/PlayerOptionProbe.obj` | Builds that object, then replays its accepted units. |
| Is the complete accepted ledger reproducible from a cold build? | `python3 scripts/analysis/verify-exact-units.py --all --json > build/accepted-unit-replay.json` | Cleans generated outputs including known VC7/linker sidecars, builds every configured object with one job, and replays all accepted units. Build logs go to stderr, so the report is pure JSON. |
| Are there exact candidates in current objects? | `python3 scripts/analysis/propose-exact-units.py --object build/NAME.obj --output .analysis/proposed-units.toml` | Read-only review artifact; accept reviewed units through `compare-function.py`. |
| What is inside the generated VC7 PDB? | `python3 scripts/analysis/inspect-pdb-streams.py build/vc70.pdb` | Lists MSF streams. Extraction requires explicit `--extract`; output defaults to `.analysis/`. |
| Are mapping overlaps explicitly classified? | `python3 scripts/validate-tracking.py --require-target` | Validates `config/mapping-overlaps.csv`; accepted nested funclets are reported separately and stale/unclassified overlap state is rejected or warned. |
| Is tracked repository state CI-clean? | `python3 scripts/ci.py` | Public, target-independent schema, documentation, and syntax checks. |
| How do I verify the active IDA database? | Follow [IDA_MCP.md](IDA_MCP.md). | Attest the GUI database before TH08 queries. |

Every public entry point supports `--help` and includes copyable examples.

When inspecting remote Actions, always name this repository explicitly:

```bash
gh run list --repo N0zoM1z0/th08 --branch main
```

A bare `gh run list` can resolve inherited upstream metadata and display
`GensokyoClub/th08` workflows instead. Confirm the repository URL and `headSha`
before recording a remote result.

## Historical reproducers

`scripts/analysis/historical/` contains reproducers for completed investigations.
The RunEcl audit, crosswalk, and shape score accompany the investigation in
[RUNECL_FUNCTION_EXACT_NOTES.md](RUNECL_FUNCTION_EXACT_NOTES.md).

## `.analysis/` lifecycle

Use ignored `.analysis/` for temporary investigation files:

1. create only inputs/results for the active bounded investigation;
2. use names containing the address or unit, not `current`, `final`, or `new`;
3. move reusable read-only logic into `scripts/analysis/` with `--help` and
   deterministic inputs;
4. record durable conclusions in a tracked focused note, ledger evidence, or
   commit message;
5. delete rejected matrices, duplicate dumps, generated objects, build logs,
   and completed handoffs before changing milestones.

Never commit executables, objects, PDBs, decompiler databases, downloaded
toolchains, or bulk generated reports.

## Validation boundaries

Before one authored exact claim, run the focused build and
`compare-function.py`, then replay every accepted unit in the affected object.
Before publishing aggregate totals, after changing a shared header/layout,
compiler flags, PCH inputs, or object graph, run
`verify-exact-units.py --all`; its default cold build is part of the evidence.
`--reuse-build` exists only for quick diagnosis. Then run
`validate-tracking.py --require-target`, `progress.py --check`, and
`git diff --check`. Before a documentation/tooling commit, run `scripts/ci.py`.
Library acceptance uses its own reviewed units, ledger, and comparator.
