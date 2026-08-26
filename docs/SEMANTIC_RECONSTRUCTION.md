# Semantic reconstruction

This phase turns layout-shaped reconstruction source into evidence-backed C++
without giving up either accepted VC7 code generation or the playable modern
ports.  Typical work replaces byte-pointer arithmetic, absolute field views,
`unk...` members, and opaque storage with named fields, aggregates, enums, and
canonical object owners.

It is not a second authored-progress ledger.  Candidate counts are routing
signals, a readable name is not proof, and preserving compilation alone does
not establish the meaning of a field.

## Evidence boundary

Keep these classes distinct in the batch note and commit message:

- **Observed:** TH08 1.00d access offset and width, signedness, bit operations,
  callers/callees, target strings, relocations, exact object code, and verified
  runtime object identity or behavior.
- **Corroborated:** several independent TH08 functions use the field for the
  same role, or TH06/TH07/upstream source names the same layout and TH08 target
  evidence agrees.
- **Inferred:** the role follows from dataflow or adjacent-version structure but
  is not independently named by TH08 evidence.  Use a neutral role name and
  state the confidence.
- **Unknown:** only storage, width, or alignment is known.  Keep it opaque; do
  not convert uncertainty into a confident-looking identifier.

IDA remains unavailable until the active database passes `docs/IDA_MCP.md`.
Semantic cleanup usually has enough source, strict VC7 replay, `typed-re.py`,
and portable runtime evidence, but lack of IDA does not lower the evidence bar.

## Select one bounded batch

Start with the read-only candidate report:

```bash
python3 scripts/analysis/report-semantic-debt.py
python3 scripts/analysis/report-semantic-debt.py \
  --category raw-member-access --details
```

The report intentionally excludes `*Probe*` files and `src/modern/` by default.
Use `--include-probes` only when a canonical exact unit still lives in probe
source.  Use `--include-modern` to audit a portability boundary, not to infer
original TH08 field semantics from backend implementation details.

Choose one structure plus one coherent field family.  Good early batches have
several independent reads/writes, a small caller set, existing exact units in
one object, and no need to change behavior.  Avoid combining unrelated renames
across managers merely because the edits are mechanical.

Before editing:

1. record the target functions and addresses that use the fields;
2. confirm those rows in `config/mapping.csv`, `config/reccmp-functions.csv`,
   and the affected units in `config/match-units.toml`;
3. inspect the current declarations, every source use, access widths, and the
   smallest relevant target fact packet or disassembly;
4. capture the affected accepted-unit baseline;
5. identify whether the declaration is shared through the PCH or by optimized
   callers, which expands the VC7 replay scope.

## Source rules

- Preserve the VC7 x86 ABI: exact field offsets, widths, signedness, packing,
  bitfield behavior, class size, construction order, and translation-unit
  visibility.
- Add `C_ASSERT(sizeof(Type) == ...)` and focused
  `C_ASSERT(offsetof(Type, member) == ...)` checks for every layout fact the
  batch relies on.  An assertion attests layout only, not the member's name.
- Prefer a real aggregate owner and a member/index expression.  Do not create a
  second global for an address inside an existing object.  The Linux port has
  demonstrated that equal symbol values without shared object identity can
  duplicate callback state and corrupt gameplay.
- Replace raw expressions with the narrowest natural typed form.  Do not mix a
  field recovery with loop cleanup, branch inversion, local reordering, or API
  redesign unless target evidence requires those changes together.
- Keep serialization buffers, instruction byte streams, tagged unions, and
  platform ABI glue byte-oriented when byte addressing is their actual
  semantics.  The candidate reporter is expected to include false positives.
- Do not rename a field solely from its numeric offset, a decompiler label, one
  visual observation, or an adjacent game's name.  Preserve a neutral name or
  opaque range until independent TH08 evidence closes the role.
- Do not add aliases, union overlays, accessors, or casts merely to make the
  source look typed.  They must express a real ownership or representation
  relationship and preserve both oracles.

## Two-oracle acceptance

The VC7 and portable builds answer different questions:

| Change | VC7 oracle | Portable oracle |
| --- | --- | --- |
| Private field rename or typed expression in one object | Build the smallest object and replay every accepted unit in that object | Compile and link the configured modern target |
| Shared header, class layout, inline body, PCH, or object-owner change | Replay every affected object, then run the cold `verify-exact-units.py --all` gate | Clean compile/link plus `verify-modern-linux.sh` when the linker layout or fixed-address ownership is involved |
| Behavior, initialization, callback state, persistence, or rendering interpretation | Strictly compare every touched function; do not accept a byte regression as a semantic improvement | Run the smallest state-transition smoke that exercises the interpretation |

VC7 exactness proves code/data identity for configured units, not the chosen
English name.  Linux success proves that the shared source remains usable on a
second implementation, not that the original binary used the same abstraction.
Acceptance requires the evidence record in addition to both applicable oracle
results.

Do not rebuild the Linux container for every spelling-only edit when an existing
configured modern tree can compile and link the same shared declarations.
Conversely, a change to target-address aliases, global ownership, initialization,
or fixed layout requires the Linux layout verifier and a relevant runtime smoke.

## Batch record

Add a concise completed entry below only after validation.  Keep experiments in
`.analysis/`; do not turn this file into a live scratchpad.

```text
### Owner / field family — YYYY-MM-DD
Scope: functions @ addresses; source files; VC7 object/profile
Observed: widths, offsets, reads/writes, calls, strings, object identity
Corroborated: independent TH08 users; labeled TH06/TH07/upstream evidence
Inference: chosen names/types and confidence; unknowns retained
Layout: sizeof/offsetof assertions added or already present
VC7 oracle: focused build/replay commands and exact unit count
Portable oracle: build/layout/runtime command and result, or why not applicable
Result: raw forms removed for this family; no repository-wide percentage claim
```

Promote a pattern to `docs/VC7_ZUN_PATTERNS.md` or
`docs/BUILD_MATCHING.md` only when it generalizes beyond the batch.  Update
`docs/RE_HANDOFF.md` only when the active family, phase, or blocker changes.

## Completed batches

### ScreenEffect variant parameters — 2026-08-26

Scope: `RegisterChain @ 0x0045B8B0`, the draw/calc callbacks at
`0x0045BB50..0x0045C0DB`, and `src/ScreenEffect.cpp` under the
`/Od /Yu"th_pch.h"` ScreenEffect profile.  `src/ScreenEffect.hpp` remains
byte-for-byte identical to the branch parent.

Observed: `RegisterChain` stores its three generic dword parameters at
`ScreenEffect + 0x18/+0x1C/+0x20`.  Full/partial/arcade fades read the first
dword as RGB color.  The `0x0045BC90/0x0045BD70` pair decrements the first
dword as a repeat count and splits alpha/RGB from the second dword.  `CalcShake`
interpolates between signed amplitudes in the first two dwords.  The
`0x0045BF10` callback treats all three as ramp-up, hold, and ramp-down frame
counts and multiplies the resulting envelope by `duration` as the amplitude.
Independent TH08 callers pass count/color pairs for effect 3, amplitude pairs
for effect 1, and three phase lengths for effect 7.

Corroborated: TH06 calls the first two slots a generic effect parameter and a
shake parameter; the bounded TH07 fade reconstruction independently identifies
the `+0x18` slot as the fade RGB parameter.  Those adjacent versions support
the stable layout, while the TH08 callbacks and callers establish the variant-
specific roles used here.

Inference: a TU-local 0x0C `ScreenEffectParameters` overlay now exposes raw
initialization, fade color, shake amplitudes, pulse repeat/color, and shake-
envelope views from one cast boundary anchored to the existing `+0x18` member.
Keeping the overlay private avoids changing the widely included VC7 header.
These role names have high dataflow confidence.  The still-unknown effect enum
names and `ScreenEffect::unk24` were deliberately left outside this batch.

Layout: assertions pin the overlay/raw sizes, its inner `+0x4/+0x8` members,
and the existing `ScreenEffect::arcadeFadeColor` anchor at `+0x18`; the existing
header assertion continues to pin `sizeof(ScreenEffect) == 0x34`.

VC7 oracle: `verify-exact-units.py --object build/ScreenEffect.obj` passed
**16 / 16**; the canonical objdiff path passed **5 / 5**.  The latter baseline
first required restoring two target-observed `g_GameManager + 0x3DBAC` DIR32
manifest entries at relocation offset 10; that ledger repair is the separate
commit immediately before this batch.

Cold aggregate diagnostic: `verify-exact-units.py --all --json` rebuilt all 75
configured objects and checked all 1,105 selected units, but returned
**1,017 / 1,105** with 88 failures outside ScreenEffect.  Repeating after
restoring `ScreenEffect.hpp` byte-for-byte reproduced the identical failure
set.  This is an aggregate branch-baseline blocker, not an aggregate exact
claim or a ScreenEffect regression; the generated report remains under
`build/accepted-unit-replay-semantic-screen-effect.json`.

Portable oracle: `scripts/build-modern-linux-container.sh` compiled and linked
the complete i386 target, then `scripts/verify-modern-linux.sh
build/modern-linux-container/th08-modern` verified ELF32/ET_EXEC/i386 and all
required fixed target-owned layout symbols.

Result: all 21 scattered raw byte-offset expressions for the three ScreenEffect
parameter slots were replaced by one asserted typed boundary plus role-specific
members.  The semantic-debt router reports zero raw-member-access candidates in
this source.  Authored/exact progress ledgers are unchanged.
