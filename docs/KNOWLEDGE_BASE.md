# Reusable knowledge map

This page routes accumulated TH08 knowledge. It is an index and contribution
policy, not another status snapshot. Search the ledgers and focused documents
before repeating target analysis or compiler-shape probes.

## Where knowledge lives

| Need | Canonical location | What it contains |
| --- | --- | --- |
| Target identity, sections, inventory classes, repository layout | [ARCHITECTURE.md](ARCHITECTURE.md) | Stable project facts and evidence boundaries. |
| Current milestone, deferred work, next bounded lane | [RE_HANDOFF.md](RE_HANDOFF.md) | Short, replaceable current state only. |
| Evidence ranking and reconstruction loop | [RE_WORKFLOW.md](RE_WORKFLOW.md) | Durable operating method and acceptance language. |
| Command selection and scratch lifecycle | [TOOLS.md](TOOLS.md) | Public entry points, copyable recipes, and tool limits. |
| Exact authored totals | [PROGRESS.md](PROGRESS.md) and `config/matches.csv` | Generated totals and accepted per-address evidence. |
| Target mappings and types | `config/mapping.csv`, `config/reccmp-*.csv` | Imported/reconciled leads; mapping alone is not exactness. |
| Canonical exact replay | `config/match-units.toml` | COFF symbol, object, extent, and explicit relocations. |
| Aggregate cold-build attestation | `scripts/analysis/verify-exact-units.py --all` and [BUILD_MATCHING.md](BUILD_MATCHING.md) | Rebuilds every configured input before replay; the durable stale-object/PCH lesson is indexed under `cold-build`. |
| Detailed VC7/comparator corpus | [BUILD_MATCHING.md](BUILD_MATCHING.md) | Address-backed build, relocation, boundary, ABI, and source-shape lessons. |
| Concise cross-subsystem compiler patterns | [VC7_ZUN_PATTERNS.md](VC7_ZUN_PATTERNS.md) | Reusable VC7/ZUN patterns with exact examples. |
| IDA/Ghidra trust boundary | [IDA_MCP.md](IDA_MCP.md) | Active-database attestation and safe fallback paths. |
| Long completed investigations | Focused `*_MATCHING.md` or `*_EXACT_NOTES.md` files | Chronological evidence and rejected probes for one bounded subsystem/function. |
| Active disposable experiments | `.analysis/` | Untracked scratch only; never current truth. |

## Existing subject index

| Subject | Start here | Notes |
| --- | --- | --- |
| ECL/RunEcl dispatch and relocations | [RUNECL_FUNCTION_EXACT_NOTES.md](RUNECL_FUNCTION_EXACT_NOTES.md) | Completed historical investigation; the final exact result is at the end. Historical reproducers live under `scripts/analysis/historical/`. |
| Player callbacks, SHT ABI, option fields | [PLAYER_MATCHING.md](PLAYER_MATCHING.md), then search `BUILD_MATCHING.md` for `Player::` | Compact ABI summary plus exact address-backed corpus entries. |
| Game manager state/setup/score | [GAME_MANAGER_MATCHING.md](GAME_MANAGER_MATCHING.md), then search `BUILD_MATCHING.md` for `GameManager::` | Focused notes are intentionally short; detailed shapes are in the corpus. |
| Pause/retry stage menus | [STAGE_MENU_MATCHING.md](STAGE_MENU_MATCHING.md) | Known draw/update family and probe ownership. |
| GUI/title/replay source shapes | Search `BUILD_MATCHING.md` for the symbol or address | Includes switch-table extents, inline ownership, table dimensionality, and frame-shape cases. |
| Function boundaries, COFF aux extents, relocations | `BUILD_MATCHING.md` | Search for `compare_size`, `COMDAT`, `REL32`, or `DIR32`. |
| Generic VC7 declaration/branch/local patterns | `VC7_ZUN_PATTERNS.md` | Search this before creating expression or `#pragma var_order` matrices. |
| Target-linked CRT/D3DX work | `RE_HANDOFF.md`, `$th08-library`, `config/library-provenance.toml`, and `scripts/compare-library.py` | Paused library foundation: archive provenance, relocation-aware match units, and a separate accepted ledger. Resume only for a bounded whole-link dependency. |
| Whole-executable reconstruction | `scripts/compare-whole-image.py`, [RE_WORKFLOW.md](RE_WORKFLOW.md), then [RE_HANDOFF.md](RE_HANDOFF.md) | Cold-build PE diff, import/resource/debug contracts, section sizes, and accepted-address link anchors. |
| Translation-unit partition candidates | `scripts/analysis/report-tu-partition-candidates.py` | Deterministic ranking by target-order inversions/drift jumps, plus bounded per-object anchor details. This is routing evidence, not a boundary claim. |
| Library candidate discovery | `scripts/analysis/propose-library-units.py` | Conservative review queue from one pinned archive; candidate status is not exact acceptance and must be promoted through an explicit unit plus `compare-library.py`. |
| Stale object/PCH exact-state failures | Search `BUILD_MATCHING.md` for `cold-build` | Why focused historical successes cannot be promoted to a current aggregate without a cold full replay. |

Fast lookup recipes:

```bash
rg -n "0x004526C0|OnUpdateHighPrioDemo2" docs config src
rg -n "compare_size|COMDAT|DIR32|REL32" docs/BUILD_MATCHING.md
rg -n "pragma var_order|frame|stack home" docs/VC7_ZUN_PATTERNS.md
rg -n "cold-build|stale object|aggregate exact" docs/BUILD_MATCHING.md docs/RE_WORKFLOW.md
```

Use the target address when names are overloaded or provisional. A logical
name alone is not a unique exact identity.

## Whole-image reconstruction lessons

The canonical first pass is a cold normal build followed by a structured PE
comparison, not a raw file hash or an unbounded library scan:

```bash
python3 scripts/build.py --fresh
python3 scripts/compare-whole-image.py --json \
  > build/whole-image-report.json
```

The following facts were observed directly in the hash-attested Japanese 1.00d
target and are safe link-contract constraints:

- the PE has no debug data-directory entry;
- import descriptors are ordered `DINPUT8`, `DSOUND`, `d3d8`, `WINMM`,
  `KERNEL32`, `USER32`, `GDI32`, `ADVAPI32`, `ole32`;
- the resource tree contains one Japanese-language 32x32 24-bit icon image and
  one group-icon leaf; the group leaf and both resource extents can be
  reproduced without copying the copyrighted target icon artwork;
- accepted authored/library addresses provide hundreds of target-to-rebuild
  link-layout anchors, but an absent map symbol is not evidence that a function
  is absent from the executable.

The following behavior was verified by rebuilding this repository: explicit
import-library order controls the resulting import-descriptor order, omitting
link-time `/debug` removes the rebuild's CodeView/debug directory, and a
build-internal icon generator can preserve tracked placeholder artwork while
emitting the target-observed resource container shape. Keep compile-time `/Zi`
separate from the final linker's debug-directory contract.

Do not infer `/OPT:REF` from the absence of a debug directory or from extra
imports. A bounded 2026-08-20 trial made the import set superficially exact but
shrunk rebuild `.text` from `0xAA26F` to `0x72B6F` and reduced located accepted
anchors from 931 to 719; the target `.text` is `0xB1B78`. The target therefore
retains code that global dead stripping removes, so the normal build keeps
`/OPT:NOREF`. This test isolated REF only; `/OPT:NOICF` remains a separate
unproven contract and must not be changed as part of the same experiment.

The 17 rebuild-only imports have bounded archive provenance. Nine are referenced
by linked D3DX `cd3dxfile.obj`/`cd3dxresource.obj`; the remaining set is
referenced by linked VC7 CRT `a_loc.obj`, `getqloc.obj`, `w_loc.obj`,
`winsig.obj`, `mlock.obj`, and `tidtable.obj`. `wopen.obj` is another archive
candidate for `CreateFileW`, but the current map does not place that member in
the link. Reproduce this distinction with
`python3 scripts/analysis/report-import-provenance.py`. It identifies later
focused link/provenance work; it does not prove that those whole members are
absent from the target or authorize deleting their imports.

Anchor drift must be interpreted structurally. If the first and last accepted
anchors from one current object have materially different drift, code outside
that object lies between them in the target layout, or the current source file
has merged target translation units. Reordering the existing object list cannot
repair such an intra-object span. First recover target translation-unit/object
ownership; only then tune object/archive order or padding. This is an inference
from link-map and target-address evidence, not yet a complete reconstruction of
the original build graph.

Generate a detailed report only after a cold normal link, then rank it instead
of manually scanning hundreds of anchors:

```bash
python3 scripts/compare-whole-image.py --json --include-anchor-details \
  > build/whole-image-anchors.json
python3 scripts/analysis/report-tu-partition-candidates.py \
  build/whole-image-anchors.json --object AsciiManager.obj
```

The first recovered boundary is the corpus example. The old
`AsciiManager.obj` linked seven exact bodies totaling `0x220` bytes between
`InitializeVms` and `RegisterChain`, while their target addresses form four
separate clusters at `0x00406FD0`, `0x00422BB0`, `0x0042F2D0`, and
`0x004398FF`. Moving the real definitions into four same-profile production
TUs made the first 21 accepted Ascii anchors, through
`PauseMenu::OnUpdate @ 0x004037B0`, land at their exact target addresses.
Uniform drift within each extracted cluster confirms that the split did not
disturb its internal layout; placement among neighboring objects remains a
separate later step. Preserve canonical match-unit object ownership and replay
both the donor and every recipient object after such a move. This proves the
necessary separation from the early donor; executable anchors alone do not
prove that each extracted cluster was a standalone original source file.

The Player pass adds the complementary case: detailed runs can mix a real TU
boundary with plain lexical definition disorder. The early bomb/shot callback
family occupies the target range `0x0040BC20..0x004142C0`, while the main
Player implementation occupies `0x00449CA0..0x00451D50` with many unrelated
subsystems between them. Moving the real early definitions to same-profile
`PlayerBomb.cpp` was therefore evidence-backed. By contrast, the main target
region is substantially continuous; creating one source file per remaining
run would have invented boundaries. Reordering the retained `Player.cpp` by
mapped target function address, with declarations for helpers now defined
after their callers, was the smaller natural repair.

Use three checks for this class of change:

1. Compare the target neighborhoods, including mapped non-anchor functions,
   before deciding whether a run reset is an object boundary or source order.
2. Move definitions rather than adding wrappers, preserve the compile profile,
   update every canonical match-unit owner plus `ghidra_ns_to_obj.csv`, and
   replay donor and recipients together. The Player move covered **116 / 116**
   accepted units and exposed the namespace-map requirement before comparison.
3. Measure the cold linked result, not just object exactness. Player improved
   from **286 inversions / 8 runs / 275,088 drift span** to a main object at
   **0 / 1 / 7,760** and an extracted object at **0 / 1 / 544**, followed by a
   full **1,105 / 1,105** cold replay.

These measurements prove the current layout improvement and the necessary
separation of the distant callback cluster. They do not prove the original
source filename or every missing callback's eventual owner.

The `main.obj` pass adds a third case: a convenient out-of-line definition can
hide consumer-emitted header COMDAT ownership. Fourteen exact
`GameManager`/`ZunTimer` helpers were appended to `main.cpp`, but `llvm-nm`
undefined-reference evidence and target neighborhoods identified natural
production consumers in `AsciiManager.obj`, `SpellCard.obj`, and
`ReplayManager.obj`. Restoring the bodies inline in their class headers made
those exact functions emit from the target-neighbor `/Od` consumers without
wrappers, copied machine code, or changing the consumers' compile profiles.
Do not select a TU merely because it implements the class: for this family the
nominal `GameManager.cpp` owner uses `/Os` and is the wrong code-generation
profile.

Use the consumer-COMDAT procedure only when all four checks hold:

1. the target body is compatible with a natural header-inline emission;
2. production undefined references identify a real caller, and its target
   neighborhood supports that caller's object/profile;
3. the body and at least one affected caller remain exact after a clean-PCH
   focused replay, followed by aggregate cold replay for the shared header;
4. the canonical match-unit owner is changed to the section-defined consumer
   object, not merely to an object with an undefined symbol of the same name.

An exact helper with no current production caller is not permission to invent
an owner. `ZunTimer::operator+= @ 0x0041FDF0` remains the sole distant
`main.obj` anchor until stronger evidence appears. Ranking metrics route an
investigation; they never justify unsupported ownership.

Resource reconstruction must preserve the evidence boundary: reproduce the
directory IDs, language, DIB dimensions/bit depth, and deterministic build
shape from repository-owned inputs, but never extract target payload bytes into
source or generated assets. A same-size resource with a different payload is an
honest remaining difference, not a reason to copy the original artwork.

Use the default summary JSON for routine work. Add
`--include-anchor-details` only when investigating a bounded object or function;
the per-anchor report is intentionally large. After any link graph, resource
generator, or linker-flag change, repeat the cold normal build before recording
new whole-image measurements.

## Promote knowledge instead of accumulating scratch

Use this lifecycle for every nontrivial investigation:

1. Put temporary disassembly, matrices, logs, and objects under `.analysis/`
   with the target address or unit in the filename.
2. Once a conclusion survives the canonical comparison, encode exact evidence
   in the appropriate ledger first.
3. Record a broadly reusable compiler/build lesson in `BUILD_MATCHING.md`.
   Distill it into `VC7_ZUN_PATTERNS.md` only when it is useful across more than
   the original function or prevents a common wrong probe.
4. Use a focused note when the evidence chain or rejected probes are too long
   for the corpus. Put a completion/historical banner at the top when done.
5. Put only the live milestone, durable blocker, and immediate next command in
   `RE_HANDOFF.md`; replace obsolete state instead of appending chronology.
6. Add a row to this subject index when a new focused note becomes a durable
   entry point.
7. Delete the superseded `.analysis/` inputs and outputs after the tracked
   knowledge and reproducible commands are committed.

Do not preserve every experiment. Keep a rejected probe only when its target
evidence and outcome will prevent a plausible, expensive repetition.

## Reusable lesson template

Use this compact structure in `BUILD_MATCHING.md`, `VC7_ZUN_PATTERNS.md`, or a
focused note:

```text
### Short pattern name
Scope: symbol @ address, compiler profile, object/unit
Observed: target instructions/layout/relocations (facts only)
Inference: source/ABI interpretation and confidence
Working shape: natural C/C++ form that reproduced the observation
Rejected alternative: only if plausible and materially different
Reproduce: exact build and comparator commands
Result: exact bytes/extent/relocations, or named remaining uncertainty
Generalization limit: where this lesson must not be copied blindly
```

An exact example should include the target address and canonical unit name.
Avoid phrases such as “current”, “final”, or “remaining” in durable corpus
entries unless they include a date/commit and are explicitly historical.

## What belongs where

- Put machine-checked exact state in CSV/TOML ledgers, never only in prose.
- Put stable project policy in `AGENTS.md` or `RE_WORKFLOW.md`, not a focused
  function note.
- Put command routing in `TOOLS.md`; do not copy competing command lists into
  every note.
- Put current work in `RE_HANDOFF.md`; do not use completed notes as a queue.
- Put executable/object/database output in ignored storage, not Git.
- Keep target observations separate from TH06/TH07 or upstream hypotheses.

## Knowledge review checklist

Before committing a knowledge update, check that:

- the target version/address and evidence class are explicit;
- the command still exists and `--help` explains its role;
- any exact claim names a reproducible comparator result;
- the lesson does not contradict the live ledgers or `RE_HANDOFF.md`;
- historical intermediate language is clearly labeled;
- another agent can find the conclusion by symbol, address, or subject index;
- replaced scratch and stale handoffs have been removed.

Run `python3 scripts/check-docs.py` and `python3 scripts/ci.py` after reorganizing
tracked knowledge.

### Library accepted rows and match units are one atomic claim

`config/library-matches.csv` must never contain an accepted row whose `unit` is absent from `config/library-match-units.toml`.  The accepted row names the claim; the unit preserves archive/member identity, target extent, and every relocation needed to reproduce it.  `validate-library.py` intentionally fails on orphan accepted rows.  If an interrupted or split commit separates the two files, restore the reviewed unit schema and rerun the canonical comparator before treating progress as current; do not weaken the validator or keep a progress-only claim.
### A zero-item conservative proposal queue is a routing result, not completion

When `propose-library-units.py --archive vc7-libcmt --min-size 1` returns zero
unique candidates, it means all candidates satisfying its strict function-aux,
size, supported-relocation, and non-relocation-byte gates have been configured.
It does **not** mean all CRT/runtime inventory is exact.  Route the remainder to
explicit boundary repair, shared/local funclet evidence, COMDAT/alias analysis,
or other archive-family investigations instead of weakening the proposer.
