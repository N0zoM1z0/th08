# Position independence in TH08

TH08 reaches position independence (PI) when code and data can move within
the executable at link time and references still reach the intended functions,
objects, and fields. For example, inserting a table before existing globals and
relinking must preserve the behavior of their readers and writers without
editing addresses by hand. ReC98 describes this in terms of replacing numeric
global addresses with symbols. It treats structure size independence as a
separate concern. [ReC98 FAQ](https://rec98.nmlgc.net/faq)

For TH08, a symbolic COFF reference can legitimately become an absolute
address in the linked PE. The linker must be able to recompute it when the
destination moves. Check internal layout independence separately from
PIE/ASLR, which concerns moving the image as a unit. Keep original member
offsets, layout assertions, and serialized file offsets where the target ABI
or file format requires them.

## Acceptance checks

1. **References follow their symbols.** Check global reads and writes, addresses
   passed as values, function pointers, and pointers in initialized data. Use
   the COFF relocation and addend to identify the destination, then check that
   the linked PE or ELF resolves it to the intended object, field, or code
   entry. Treat numeric address scans as leads and classify each candidate by
   its use. Addresses recorded in mapping and comparison inputs describe the
   reference executable and are expected there.
2. **Shared storage keeps its identity.** When original code uses the same
   bytes through several names, verify that every linked reader and writer
   reaches the same storage. Include member views inside larger objects.
   Check that initialized objects still receive their expected contents after
   linking. A name bound to a retail virtual address in a production linker
   script is still a fixed-placement dependency.
3. **Bulk operations respect object boundaries.** A clear or copy that spans
   several fields must operate within an explicitly owned range. When the
   operation spans distinct globals, establish how the build guarantees the
   required relationship or verify that it handles the objects separately.
4. **Exercise a changed link layout.** Use an isolated build and move the
   relevant code, initialized data, or zero-initialized data. Check the map or
   symbol table to confirm the intended objects moved. Exercise the affected
   initializer, callback, state transition, or persistence path and compare
   its results with the expected behavior. Keep the standard VC7 comparison
   against Japanese TH08 1.00d and verify the affected code, relocations, and
   declared data under the existing evidence rules. [Exact evidence](EXACT_EVIDENCE.md)

Record PI evidence per build product and scope. State which references and
initialized data were checked, which objects moved, and what behavior was
observed. A whole-image claim also covers linked library code and data. On
`main`, report the VC7 reconstruction and Linux i386 product separately. The
native 64-bit port has its own build and layout evidence.

## TH08 cases to check

| Case | Expected result after relinking |
| --- | --- |
| `EnemyManager::AddedCallback` at `0x0042EBF0` | The ECL manager and call parameters are reset as intended, and the neighboring effect manager is untouched. |
| GUI and Supervisor at retail `0x017CE8CC` | Dialogue writes, HUD reads, and the frame countdown use the same storage. |
| `GameManager::hscr` counters at retail `0x0164CF9C`, `0x0164CFA8`, and `0x0164CFAC` | The collection and bomb paths update the fields that the results screen copies. |
| Linux i386 stage music contexts at retail `0x004C7240` | The initialized song numbers reach the music unlock path when the table is moved. |

## Linux i386 link experiment

The Linux product now links native game globals and tables through the ELF
sections. The stage music table uses its `Gui.cpp` initializer, which restores
the song numbers omitted by the old fixed-address startup path. The effect
table uses the gameplay definitions for its callback pointers.

The ordinary container build and a second link with the same objects in
reverse order both pass `scripts/verify-modern-linux.sh`. To repeat the second
link, take the link command from `ninja -t commands th08-modern` in the i386
build directory. Reverse its game object arguments, keep the libraries in
their original order, and write the result to a separate output. In the
observed build, `g_EclRuntime` moved from `0x007C9FE0` to `0x00E25140`, the
stage music table moved from `0x000F0160` to `0x000F1100`, and the enemy
registration callback moved from `0x00064B20` to `0x0009F7A2`. These are ELF
symbol values before load bias.

Both links passed the asset-free ECL reset and shared-field checks. At entry
to `main`, all 27 stage music entries matched the original table, and sampled
spell, bonus, dialogue, and effect data had their expected values. Both
executables ran from the title into the bundled Stage 5 demo and loaded its
SHT, STD, ECL, and message files. The verifier rejects the original fixed
layout and a PIE carrying an absolute TH08 symbol.

## Windows i386 link experiment

The normal VC7 build passes the cold accepted-unit replay. The playable
VC7 `bugfix` build was also linked with its game objects in reverse order,
using the same libraries. The `/out` argument of the Ninja link command
selected a separate PE and map. The regular and reordered links moved code,
initialized data, and mutable state within the image.

| Symbol | Regular link | Reordered link |
| --- | --- | --- |
| `EnemyManager::AddedCallback` | `0x00420160` | `0x0045AE60` |
| `g_GuiStageMusicContexts` | `0x004D45B0` | `0x004D4300` |
| `g_EclRuntime` | `0x004F9F18` | `0x018C4FE0` |
| `g_EffectManager` | `0x004FA0A0` | `0x01839F40` |
| `g_GameManager` | `0x0161C790` | `0x00764AF0` |

The normal PE and both `bugfix` links resolve the checked ECL, Supervisor,
and GameManager operands to their owning objects and members. The ECL reset
range stays disjoint from the effect manager. All declared initialized-data
contracts pass in the reordered PE. Under Wine, the reordered build completed
`easy-0-b` through Final B and agreed with the attested retail recording on all
25 recorded fields for 107,296 calculations. Use `test-replay-suite.py` with
the reordered executable and its paired map to repeat the comparison.

These experiments cover the listed owners, data contracts, and exercised
runtime paths. A whole-image PI claim requires evidence for the remaining
initialized data and linked library paths.

## Solver replay validation

`th08-solver` at `bf782474` generated two Magic Team Lunatic Stage 1
Practice replays with the existing seed `23035`. One run used item guidance
and required all captures. The other disabled item guidance and made
captures optional. Both re-executed their input tapes to the same terminal
state in a fresh solver process. The published Scarlet Team Extra replay
with seed `9098` supplied a separate route with a different shot and seed.

All three replays reached their natural end on retail 1.00d, the reordered
VC7 `bugfix` build from `3669a7fb`, the reordered x86_64 build from `633e81ee`,
and the solver's headless playback. Each agreed with retail on all 25
recorded fields, including the exact float bits, and on the stage-end score.

| Replay | Calculations |
| --- | ---: |
| Stage 1 with item guidance | 11,068 |
| Stage 1 without item guidance | 13,508 |
| Published Scarlet Extra | 76,721 |

The solver used GCC 13. A standalone `std::filesystem::absolute` program
reproduced the startup crash seen with the host's GCC 8 build. Generation
used one CPU with a 4 GiB hard memory cap. The two runs took 96 and 120
seconds and each peaked below 74 MiB across its process tree. The local
replays, retail traces, comparison reports, and measurements are under
`build/solver-replay-audit/`. The x86_64 captures are in the same relative
directory in the portable worktree.

## Silent Selene replay validation

Ten selected Silent Selene replays covered all five difficulties, both
Stage 4 branches, Final A, Final B, Extra, and a tool-assisted Extra run.
The original 1.00d executable and the same reordered VC7 and x86_64 builds
used above agreed on all 25 recorded fields over 769,880 calculations.
All ten reached their natural end and matched every recorded stage-end
score. The selection, hashes, and per-replay results are in the local
`build/silent-selene/summary.json`.

Four runs ended at game over with four or five input records remaining in
the final stage. The original executable produced the same complete trace
on a second capture for each file. Those checks pinned the observed tail
length to each replay hash. The repository's published replay corpus keeps
its existing acceptance rules.
