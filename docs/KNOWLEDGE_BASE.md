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
| Semantic field/type recovery and two-oracle acceptance | [SEMANTIC_RECONSTRUCTION.md](SEMANTIC_RECONSTRUCTION.md) | Evidence classes, bounded batch format, typed-layout rules, and VC7/portable validation matrix. |
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
| Raw offsets, anonymous fields, and semantic naming | [SEMANTIC_RECONSTRUCTION.md](SEMANTIC_RECONSTRUCTION.md), `$th08-semantic`, then `scripts/analysis/report-semantic-debt.py` | Candidate scans are routing only. Accept one field family from target evidence plus applicable VC7 and portable oracle results. |

Fast lookup recipes:

```bash
rg -n "0x004526C0|PlaybackExtendedInputAndFps" docs config src
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
`/OPT:NOREF`.

ICF is independently target-proven and must remain enabled. Target constructor-
iterator sites in Background, ECL, Enemy, Gui, ScreenEffect, and AnmManager all
load `0x0040B580` for layout-identical vertex element constructors that the
current COFF objects expose under four different decorated symbols
(`BackgroundStageVertex`, `VertexTex0Xyzrhw`, `VertexTex1DiffuseXyzrhw`, and
`VertexDiffuseXyzrhw`). VC7 `/OPT:ICF` reproduces that cross-symbol shared RVA;
`/OPT:NOICF` leaves four separate bodies. The target also contains the already
reviewed `CSoundManager` inline-constructor fold at `0x004716E0`. Keep
`/OPT:NOREF` and `/OPT:ICF` as separate, evidence-backed link contracts.

The 17 rebuild-only imports have bounded archive provenance. Nine are referenced
by linked D3DX `cd3dxfile.obj`/`cd3dxresource.obj`; the remaining set is
referenced by linked VC7 CRT `a_loc.obj`, `getqloc.obj`, `w_loc.obj`,
`winsig.obj`, `mlock.obj`, and `tidtable.obj`. `wopen.obj` is another archive
candidate for `CreateFileW`, but the current map does not place that member in
the link. Reproduce this distinction with
`python3 scripts/analysis/report-import-provenance.py`.

The D3DX side is now version-bounded rather than speculative. In the pinned
Visual Studio .NET 2002 prerelease `D3DX8.LIB`, `obj/i386/d3dx8tex.obj` contains
auxless code COMDATs whose complete section extents exactly equal the target:
`D3DXLoadSurfaceFromFileInMemory @ 0x0047AAF3` is 0xB8 bytes with four REL32
relocations and `D3DXCreateTextureFromFileInMemoryEx @ 0x0047B72E` is 0x40
bytes with one REL32 relocation. Replaying those relocations leaves zero byte
differences, so both are accepted library units. The same archive member also
contains file/resource wrapper functions whose undefined references pull
`cd3dxfile.obj` and `cd3dxresource.obj` into a `/OPT:NOREF` link. A controlled
probe with `scripts/prefix/mssdk/lib/d3dx8.lib` did not remove any of the 17
extra imports, shrank `.text` raw size from `0xAA400` to `0x96C00`, reduced
located anchors to 930, increased missing anchors to 86, and uses a 0x411-byte
`D3DXCreateTextureFromFileInMemoryEx` section instead of the target 0x40-byte
wrapper. Reject that archive substitution. The remaining D3DX import mismatch
is therefore an archive-member retention/splitting question under the required
`/OPT:NOREF`, not evidence for changing the production AnmManager API calls or
for enabling global dead stripping.

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

The `AnmManager.obj` pass adds the guardrail for optimized consumers. Moving a
helper body to a header is a caller-codegen change even when the standalone
COMDAT remains exact. A focused five-object replay initially passed all 260
units, but the required full cold replay exposed five Title regressions after
`AnmVmBase::Initialize` became visible to `/Os /Ob1`. Keeping its declaration
in the header and placing the exact body explicitly in target-neighbor
`AsciiManager.cpp` restored all optimized callers. Expand the focused set to
every optimized production/probe caller when changing inline visibility; the
aggregate replay is still the acceptance gate.

The reverse case is equally important. `AnmManager::SpriteHasTexture @
0x004622C0` appears late in the current Anm object when emitted from the class
header. Making it out-of-line beside its Anm caller removed all 20 remaining
Anm inversions, but `TitleScreen::OnDraw @ 0x0047087F` became 59 bytes short:
the target has the helper inlined and no call relocation. Preserve the proven
inline caller contract even when its standalone COMDAT placement worsens a
routing metric. The accepted Anm repair therefore combines seven
target-neighbor helper owners with lexical reordering of the continuous render
cluster, improving **195 / 10 / 400,208** to **20 / 2 / 359,840** without
inventing source-file boundaries.

The `Global.obj` pass adds the explicit shared-helper case. Six ordinary math
definitions were interposed in the current Global object even though target
neighborhoods and real production references put four beside Background code
and two beside Player bomb code. Because every involved TU uses `/Od`, moving
the definitions directly to those target-local lexical positions recovered
the consumer ownership without changing shared-header visibility. The retained
Global range was continuous; ordering `Chain::RunDrawChain` before
`Chain::ReleaseSingleChain` was therefore a lexical repair, not evidence for a
new TU. Together these changes improved Global from **194 / 4 / 220,816** to
**0 / 1 / 96**, with a **191 / 191** focused replay and **1,105 / 1,105** cold
aggregate replay.

A helper move can also change which implicit COMDATs the donor happens to
emit. Moving those math bodies removed `Float3::Float3(float,float,float) @
0x00404720` from Global even though its source had not changed. Reinspect the
section-defined symbols in the donor and recipients after every ownership
move. In this case the target constructor sits beside `PauseMenu::OnDraw`, and
`AsciiManager.obj` naturally emits the exact constructor, so Ascii became the
canonical evidence owner. Treat this as a cascading ownership correction, not
permission to route a missing symbol to whichever object happens to define a
byte-identical copy.

The `SoundPlayer.obj` pass demonstrates paired consumer emission. The target
alternates each small `SoundPlayer` fade wrapper with the
`CStreamingSound` helper it calls at `0x00406AC0..0x00406BE0`, followed by the
Sound pause commands. `AsciiManager.obj` is the sole production consumer of
the outer wrappers, and restoring both wrapper and nested helper bodies inline
made VC7 emit exactly that pairwise order under Ascii's `/Od` profile. When a
target cluster alternates forwarding wrappers with their callees, test natural
consumer-triggered header COMDATs before inventing a helper source file.

The retained Sound tail shows the complementary safe out-of-line case. Three
tiny accessors were deferred after explicit functions even though the target
places them first. Their complete caller set is confined to the same `/Od`
Sound TU, and exact caller relocations prove real call boundaries. Replacing
their header bodies with declarations and defining them lexically before the
constructor restored **0 inversions / 1 run / 0 span** without changing caller
codegen. Removing inline visibility is still a shared-header change: focused
replay is diagnostic, and the cold **1,105 / 1,105** aggregate replay is the
acceptance gate.

The `GameManager.obj` pass is the combined-case reference. One ranking
candidate contained ordinary lexical disorder, natural consumer COMDATs,
target-local explicit cross-subsystem definitions, and one stale objdiff-only
owner. Treating every reset as the same kind of boundary would have produced
the wrong architecture.

Use these distinctions:

1. If header helpers form a target-neighbor group and a real production caller
   emits exact copies, restore the inline bodies and canonicalize the
   section-defined consumer. The GameManager setter group belongs to
   `AsciiManager.obj`; the solo-character predicates and `GetLives` belong to
   `EnemyManager.obj`.
2. PCH include order can determine the relative order of deferred VC7 COMDATs.
   The target early cluster places Sound fade/pause helpers before the
   GameManager setters, so `th_pch.h` must include `SoundPlayer.hpp` before
   `GameManager.hpp`. After changing this order, clean the PCH and replay every
   accepted unit, not just the moved helpers.
3. If the retained target neighborhood is continuous, reorder real definitions
   instead of splitting it. A deferred header body may become explicit when
   target order and complete caller evidence support the call boundary;
   `AddLives` belongs between the GameManager constructor and
   `InitArcadeRegionParams`.
4. A target-local block can contain definitions from more than one class.
   `Player.cpp` explicitly owns `SetYoukaiGauge`, `RandomizeAntiTamper`,
   `AddToDeaths`, and `AddToBombsUsed` in the exact
   `0x0044E140..0x0044E348` order. This is stronger than leaving exact bodies
   in a nominal class donor or a private objdiff object because Player has the
   production calls and the mapped neighboring functions.
5. An already recovered target-contiguous production cluster may absorb its
   missing leading definition when target order, compile profile, and exact
   replay all agree. `ScaleFloatBasedOnRank @ 0x00422B80` followed by the two
   Ascii boss-marker functions at `0x00422BB0..0x00422C13` is exact as one
   three-anchor block; no padding or artificial reference is needed.

This pass improved `GameManager.obj` from **37 anchors / 174 inversions / 9
runs / 291,648 drift span** to **30 / 0 / 1 / 3,970**. The Player-side block
also leaves `Player.obj` at **36 / 0 / 1 / 7,264**, while the three-function
boss-marker block is **3 / 0 / 1 / 0**. Focused donor/recipient replay plus the
cold **1,105 / 1,105** aggregate replay are both required; a perfect ordering
metric does not replace exact-code attestation.

The `Gui.obj` pass is the clean retained-TU lexical-order case. Forty explicit
Gui-region definitions cover one substantially continuous target sequence, so
reordering the real definitions by mapped target address reduced the object
from **167 inversions / 12 runs / 23,133 span** to one residual inline-COMDAT
reset without changing function behavior. Necessary forward declarations are
preferable to wrappers when an earlier caller now precedes a file-local helper.

That residual also refines the inline-ownership rule.
`AnmVm::FUN_004396f8 @ 0x004396F8` had the correct production owner (`Gui.obj`)
but header visibility forced VC7 to emit it at first use. Its production caller
set is Gui-only, while two exact PlayerOption probe callers prove real REL32
call boundaries. Converting the header body to a declaration and placing the
unchanged explicit body at the target-local Gui position preserved both caller
families and reduced `Gui.obj` to **0 inversions / 1 run / 101 span**. A
correct owner does not imply a correct emission position; when callers are
bounded and target relocations prove out-of-line calls, explicit same-owner
lexical placement is a valid repair.

The `SpellCard.obj` pass is a bounded residual example. Sorting all explicit
definitions by target address preserved **34 / 34** focused exact units and
reduced the object from **122 inversions / 7 runs** to **18 / 2**, but the last
reset is not safe to remove mechanically. Exact consumer COMDATs for
`ZunTimer::operator--(int)` and the GameManager spell-number predicates target
`0x00418110..0x00418180`, while several explicit Spellcard helpers belong to
far-later target neighborhoods and currently have no production caller that
proves a new owner. A strong lexical metric improvement is useful evidence, but
stop when eliminating the remainder would require changing shared inline
visibility or manufacturing an unsupported TU owner.

The `BulletManager.obj` pass reinforces the retained-TU lexical rule across a
wide target span. The current source mixed the early clear wrapper, constructor/
manager blocks, bullet update helpers, and lifecycle tail out of target order.
Sorting the existing real definitions by mapped address, plus two ordinary
forward declarations, preserved **38 / 38** focused exact units and reduced the
object from **85 inversions / 4 runs** to **0 / 1**. A large drift span can
remain even after order is correct when one current object legitimately contains
widely separated target neighborhoods; inversion/run metrics and drift span
answer different questions.

The `Background.obj` pass shows why target-neighbor caller evidence can be
stronger than nominal class ownership. `Background::background_fun_00415ce0`
and `background_fun_00416ad0` are called only from production Spellcard code,
and their target addresses sit inside the corresponding Spellcard lexical
sequence. Moving the unchanged bodies to `Spellcard.cpp` and canonicalizing the
recipient object preserved both donor and recipient exactness. For the retained
Background TU, use targeted block moves when file-scope data declarations are
interspersed with functions; a mechanical whole-file function sort can break
visibility even when the desired code order is correct. This combination
reduced Background from **61 inversions / 6 runs** to **0 / 1**.

The `Midi.obj` pass is the same-owner deferred-body case in a shared header.
`MidiTimer::OnTimerElapsed @ 0x0043EF40` and `MidiOutput::Ntohl @ 0x004444E0`
were already naturally owned by Midi, but inline/header definitions emitted at
late first-use positions and created the sole target-order reset. Their caller
sets are bounded to Midi and target code preserves actual call/virtual
boundaries, so declaration-only headers plus unchanged explicit definitions at
the target-local positions preserve exact callers while changing only emission
order. Focused **33 / 33** and cold **1,105 / 1,105** replay reduced Midi from
**43 inversions / 2 runs** to **0 / 1**. Treat same-owner emission placement as
a separate question from semantic ownership.

The `EnemyManager.obj` pass combines function-only sorting with an existing
target-contiguous recipient cluster. When file-scope statics are interspersed
with functions, extract and reorder function bodies only; leaving the data
declarations fixed avoided the failure mode seen in a whole-file Background
sort. After that lexical repair, the only residual was a pair of Gui helpers
that target places immediately after the recovered `AsciiManagerScale` block
and immediately before BulletManager. The donor and recipient share `/Od` PCH
codegen, and moving the unchanged bodies made `AsciiManagerScale.obj` a strict
**4 / 0 / 1 / 0** target-order block while EnemyManager reached **0 / 1**. A
previously recovered contiguous cluster can absorb an adjacent cross-class
definition when target sequence, compile profile, focused replay, and cold
aggregate replay all agree; class name alone is not the production-TU contract.

The `Supervisor.obj` pass shows that a shared-header body can safely become an
explicit same-owner definition even with multiple optimized callers, but only
when target relocation evidence proves those callers are already out-of-line.
`GameManager::IsExtraUnlockedWithAllTeams @ 0x00447D04` is called from several
TitleScreen paths; their accepted units contain REL32 calls, and declaration-
only visibility preserved **23 / 23** TitleScreen plus **11 / 11** probe units.
`SoundPlayer::UpdateFades @ 0x00447764` is the simpler single-production-caller
version of the same pattern. Explicit target-local placement plus one ordinary
`UpdatePlayTime`/`UpdateGameTime` swap reduced Supervisor from **29 / 4** to
**0 inversions / 1 run** without changing any helper owner.

The `ReplayManager.obj` pass is a compact reference for mixing ordinary lexical
repair and same-owner header deference in one TU. Move large real definitions
(`LoadReplayData`, constructor) by mapped target address first; then repair the
small deferred tail only when the production caller set is bounded. Here both
`GameManager::SetClockTime @ 0x00453C60` and `ReplayManager::IsDemo @
0x00453CC0` are Replay-only calls, so explicit target-local bodies after
`AppendFormat` preserved exactness and reduced **23 inversions / 4 runs** to
**0 / 1**.

The `PbgFile.obj` pass extends the same-owner deferred-body rule to base-class
construction. An inline empty base constructor/destructor can be emitted at the
first derived construction site even when the shipped object places the base
standalone COMDATs after the entire derived implementation. If the target order
is contiguous and focused replay proves the bodies unchanged, declaration-only
base methods plus explicit same-TU definitions can restore production order
without changing vtable/caller behavior. `IPbgFile` moved to the target
`0x00473C40..0x00473C60` tail, taking PbgFile from **20 inversions / 2 runs**
to **0 / 1**.

The `EffectManager.obj` pass shows a safe way to normalize a source file whose
functions are extensively interleaved with file-scope declarations. Extract
only real function blocks, preserve all non-function declarations/data in their
original relative order, then append the function blocks in mapped target order.
Compile visibility failures should be solved only with real forward declarations.
For EffectManager, two prototypes were sufficient; focused **52 / 52** plus the
cold aggregate replay reduced **17 inversions / 4 runs** to **0 / 1** without an
ownership change.

The `ResultScreen.obj` pass adds an optimization-profile rule to target-neighbor
rehome. `ScoreListNode::ScoreListNode @ 0x0045A4DC` is target-contiguous with
`ScoreDat` but moving it into ordinary `/Od` `ScoreDat.cpp` changed its exact
extent from 0x20 to 0x2B bytes. The same unchanged body became exact when it
shared the immediately preceding `Catk::WasAttemptedWithShot` local
`#pragma optimize("s", on)` region. A target-contiguous owner move may therefore
require preserving the donor's codegen profile locally inside the recipient TU;
address adjacency alone is insufficient. In the same pass,
`AnmManager::ReplaceSurface @ 0x0045A3FD` was a simpler single-caller
same-owner deferred-header repair in `ResultScreen.cpp`.

The `PbgArchive.obj` pass is another base/helper deferred-emission case: header-defined `CPbgFile::ReadInt @ 0x004751E0` and `PbgArchiveEntry::~PbgArchiveEntry @ 0x00475270` were exact but emitted at their first users. Declaration-only headers plus unchanged explicit bodies at the target tail restored **0 inversions / 1 run** without changing ownership or behavior.

The `TitleScreen.obj` pass confirms that target-late inline helpers can be made
explicit when every affected production/probe caller remains byte-exact:
`SetKeyNumberSprite @ 0x00469FA9` and
`GameManager::IsLastWordSpellCardAttempted @ 0x0046FD5F` both survived their
TitleScreen and TitleReplay caller gates. `MusicRoom.obj` extends this into a
caller-preserving COMDAT routing pattern. A global declaration-only change for
`Supervisor::IsMusicPreloadEnabled @ 0x00449C79` changed two exact Supervisor
callers by 1-2 bytes. Instead, make only the desired consumer TU see a
declaration (with a TU-local preprocessor gate), keep the original inline header
body visible everywhere else, and provide an **inline explicit definition** at
the target-local consumer position. The explicit `inline` is essential: without
it the cold link reports LNK2005 against the other header COMDAT copies. With it,
all callers retain their original codegen while production object order selects
the target-local COMDAT. This reduced MusicRoom to **0 inversions / 1 run / 0
span** with MusicRoom 11/11, Supervisor 49/49, SoundPlayer 25/25, and cold
aggregate 1,105/1,105 exact.

The `ScreenEffect.obj` pass is the minimal lexical case: a single pair of exact explicit functions (`DrawPartialFade` / `DrawArcadeFade`) was reversed in source. Swapping only those function blocks reduced **1 inversion / 2 runs** to **0 / 1** while preserving **21 / 21** focused exact units.

The production ECL-owner pass adds a target-lane ownership rule. Nominal class
ownership is weaker than a continuous target subsystem lane when several exact
helpers were temporarily reconstructed in unrelated production TUs. The target
places `EclManager::RunEcl @ 0x004184B0` before the ECL helper neighborhood at
`0x0041F000..0x0041FDF0`; `EnemyManager.cpp`, `Spellcard.cpp`, `main.cpp`, and
`EclManager.cpp` all use the same `/Od /Yu"th_pch.h"` profile. Rehoming only the
unchanged exact helper bodies and their canonical match-unit owners into
`EclManager.cpp` preserved all donor and recipient focused units and cold
**1,105 / 1,105**, while reducing both SpellCard and main to **0 inversions / 1
run**. Do not generalize from class names alone: require a mapped target lane,
compatible codegen, exact donor/recipient replay, and a cold linked-order win.
This does not by itself promote the separately accepted probe `RunEcl` body to
production ownership; helper-lane recovery can proceed incrementally.

The `AnmManager.obj` pass refines caller-preserving COMDAT routing for PCH
consumers. A TU-local preprocessor gate cannot hide a class-inline body from a
`/Yu` source when that class definition is already stored in the PCH. In that
case, make the shared header declaration-only, put an `inline` COMDAT definition
at the desired owner position, and restore equivalent early body visibility in
any optimized caller TU that needs it. `SpriteHasTexture @ 0x004622C0` used
this PCH-safe form; AnmManager **83/83**, TitleScreen **23/23**, and cold
**1,105/1,105** remained exact while the object reached **0 inversions / 1
run**.

The final `AsciiManager.obj` pass shows that a dense PCH/COMDAT residual should
be decomposed by emission mechanism rather than treated as one owner problem.
An ordinary target-local definition fixed `Float3 @ 0x00404720`; changing
`AnmVmBase::Initialize @ 0x004068E0` to `inline` moved it into the COMDAT queue;
reordering unchanged ZunTimer and GameManager class-member definitions repaired
their emitted lexical order; and the target-neighbor `AsciiManagerGauge.obj`
absorbed the standalone `0x00407140..0x00407180` helpers after exact REL32
caller evidence proved that header bodies were unnecessary. Moving the `Float3`
constructor out of its shared header also demonstrates a cold-build caller gate:
focused production replay can remain exact while other optimized/probe TUs lose
the former inline compilation context. Restore the same unchanged body as an
early TU-local `inline` definition in those callers rather than returning the
canonical owner to the header. Supervisor, TitleScreen, and TitleReplay required
that treatment; the final cold corpus then returned to **1,105 / 1,105** while
Ascii stayed at **0 inversions / 1 run**. Prefer the smallest
mechanism-specific repair for each reset; do not globally toggle header
visibility when lexical order or a bounded recipient owner is sufficient.

The layout closure check must compare the production link object set against the
TU report, not just trust the ranked list. The final link has 36 production
objects; 35 have at least one accepted function anchor and all 35 are **0
inversions / 1 run**. `EclGlobals.obj` is the only unranked object and is a
data-only TU with no function definitions. Treat that as explicit closure of the
function-layout inventory rather than silently ignoring an unranked object.

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
