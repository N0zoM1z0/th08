# Native Windows i386 reconstruction runtime

This is the prerequisite whole-program validation lane for the reconstructed
Japanese TH08 1.00d source. It builds the production translation units with the
pinned Visual C++ .NET 2002 toolchain, links a real PE32 GUI executable, and
runs that executable with the original game data on Windows.

It must precede modern Windows/Linux/macOS port work. A modern compiler can
accept different declarations, choose different owners, add compatibility
initialization, or tolerate a link graph that the VC7 production image does
not. A modern executable can therefore be useful portability evidence while
still hiding a missing VC7 data owner, translation-unit boundary, static
initialization, callback ABI, or lifetime defect.

## Artifact boundary

| Artifact | Purpose | What it can prove |
| --- | --- | --- |
| `resources/th08.exe` | Canonical Japanese 1.00d target | Target bytes and behavior; never modified or distributed |
| `build/th08.exe` | Native VC7 production reconstruction | Native i386 compile, link, ownership, initialization, and runtime behavior |
| `th08-modern.exe` or a Linux/macOS binary | Portability product | Behavior under its own compiler/backend; cannot substitute for `build/th08.exe` |
| preserve-lives launcher | Process-only endurance aid | Wider runtime coverage for one hash-pinned reconstruction; no correctness or exact-match credit |

Do not make the native image runnable with unresolved-symbol forcing, blanket
aliases, duplicate storage, fixed preferred-base function addresses, fake
returns, or modern startup `memcpy` initialization. Repair the real semantic
owner, ABI, translation unit, link input, or lifetime instead.

## Reproducible build and static gates

First verify the private target and current ledgers:

```bash
python3 scripts/verify-target.py resources/th08.exe
python3 scripts/analysis/report-reconstruction-status.py --summary
```

Cold-build the complete production image with one VC7 job:

```bash
python3 scripts/build.py --build-type normal --fresh -j 1
```

The link must produce `build/th08.exe` without unresolved-symbol forcing. The
output is expected to be a PE32 i386 GUI executable; merely producing a file is
not a runtime pass.

For the initialized-data families recovered by this phase, run:

```bash
python3 scripts/analysis/verify-windows-i386-runtime-data.py
```

That check verifies the target hash, rejects mapped uninitialized production
owners in target raw-backed sections, compares the Last Spell count, stage
bonuses, and dialogue palettes byte-for-byte, and resolves all 66 Effect-table
callback pointers through the current linker map.

Source or shared-owner changes also require the normal exact gates:

```bash
python3 scripts/analysis/verify-exact-units.py --all
python3 scripts/ci.py
git diff --check
```

The `--all` replay is intentionally cold and single-job. A reused object tree
cannot support an aggregate exact statement after owner, header, compiler
profile, or link-graph changes.

## Isolated Windows deployment

Never test by overwriting the canonical installation. Create a separate
directory, for example:

```text
D:\Entertainment\Game\Touhou\th08-reconstruct
```

Copy `build/th08.exe` there as `th08-reconstructed.exe`, then copy the legally
owned runtime data (`th08.dat`, `thbgm.dat`, and any other files required by the
original installation). Do not commit those files. Preserve old executable,
log, replay, score, and crash artifacts when changing builds so a failure can
be tied to the exact executable hash that produced it.

Copy `scripts/run-windows-i386-reconstruction.bat` beside the executable for a
normal run, or copy both `run-preserve-lives-test.*` files for the endurance
mode described below. These launchers intentionally pass no modern-port
arguments.

TH08 stores fullscreen/windowed selection in `th08.cfg`; the reconstructed VC7
image does not need a modern command-line override. Use a known windowed
configuration in the isolated directory for compatibility testing. Keep the
original installation and its configuration untouched.

Before replacing or launching the executable, make sure no prior TH08 process
is running. Do not rebuild with Wine/VC7 while a Windows-host runtime test is
active; this repository uses one writable build/runtime session at a time.

## Runtime matrix

A minimum manual pass should exercise ownership and lifetime boundaries, not
only reach the title screen:

1. start in windowed mode and enter gameplay with the copied retail data;
2. use Sakuya's and Remilia's X bombs and compare the complete effect paths;
3. die normally, observe death effects, item/power changes, respawn, and the
   life display;
4. return from gameplay to the title, start another run, and repeat;
5. visit Music Room, Options, practice/story menus, replay selection, and
   return paths;
6. cross stage and dialogue transitions, including enemy-name and portrait
   texture replacement;
7. save and replay a run when the preceding paths are stable; and
8. close the game normally and record whether the process exits unexpectedly.

Track every observation in [the runtime issue ledger](RUNTIME_ISSUES.md).
Record the executable SHA-256, exact interaction, last visible frame, whether
Windows still reports the process alive, and any crash/event-log evidence.

## Preserve-lives endurance launcher

`scripts/run-preserve-lives-test.bat` and its PowerShell helper are
intentionally narrower than a "no-death" patch. The player must still enter
the normal death, effect, power drop, respawn, invulnerability, replay-event,
score, and UI paths. The launcher changes only the immediate argument of the reconstructed
`GameManager::AddLives(-1)` call to zero in the live process, so the call and
all surrounding bookkeeping still execute without consuming a life.

The helper must be regenerated or reviewed after every executable change. It
is pinned to a SHA-256, a map-derived RVA, and expected instruction bytes; it
must refuse the canonical target and every unknown reconstruction. It never
modifies the executable on disk and is never a production or release mode.

## Acceptance language

- A successful VC7 compile/link is a **native production build**, not a runtime
  pass or a whole-image exact claim.
- A manually exercised interaction is a **runtime observation** tied to one
  artifact hash.
- A repaired issue is **fixed / confirmation pending** until the originally
  reported path has been repeated on the repaired native image.
- Function exactness remains governed only by the configured target comparator.
- A runtime repair does not add exact credit, and a function-level exact result
  does not prove correct whole-program ownership or lifetime.
