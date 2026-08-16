# RunEcl function-exact notes

Target: Japanese TH08 1.00d `resources/th08.exe` at `0x004184B0`.

These notes are for continuing `th08::EclManager::RunEcl` from shape-exact
status toward strict `compare-function.py ecl-manager-run-ecl` exactness.  They
record reproducible observations and failed natural C++ probes so later workers
do not have to rediscover them.

## Current baseline

As of `a553062 gpt-web: align runecl context selection flow`:

```text
scripts/ecl-shape-score.py --object build/probes/EclRun.obj
function=0x6B06/0x6B06
code=0x680E/0x680E
physical_delta=0
positive_delta=0
absolute_delta=0
max_positive_delta=0
```

This is **shape / extent exact**, not strict function exact.  The strict
comparator still fails before byte comparison unless a relocation manifest is
provided, because the RunEcl unit currently has no `[[units.relocations]]` rows
while the COFF object has hundreds of real relocations.

Do not call this function exact until:

```bash
python3 scripts/compare-function.py ecl-manager-run-ecl --json
```

returns `"result": "exact"` with the canonical manifest.

## Temporary strict-compare workflow

For local investigation only, generate a temporary relocation manifest from the
current COFF relocation table by using the target field at the same offset to
infer the destination address.  This allows `compare-function.py` to get past
its relocation-key gate and report byte differences after relocation replay.

A local helper under `.analysis/` is appropriate; do not commit the generated
manifest until byte replay is exact and every relocation target/symbol has been
reviewed.  `$L...` compiler local labels must be normalized to `$L*`, matching
`compare-function.py`'s stable-symbol rule.

Current byte-replay baseline with a temporary manifest:

```text
result=mismatch
reloc_count=799
diff_count=5033
matched_bytes=21605/26638
matched_compared_bytes=22365/27398
```

First diff is the prologue frame allocation:

```asm
target: sub esp, 0x5C4
object: sub esp, 0x5C0
```

## Byte-diff distribution

With relocations replayed, current largest byte-diff contributors are:

```text
op3   diff_bytes=621  size=0x4DA  aliases=[3,84,85]
op72  diff_bytes=350  size=0x1F0
op127 diff_bytes=324  size=0x1E4
op79  diff_bytes=239  size=0x134
op114 diff_bytes=178  size=0x350 aliases=[114,115]
op66  diff_bytes=177  size=0x110
op80  diff_bytes=139  size=0x12A
op81  diff_bytes=139  size=0x12A
op87  diff_bytes=130  size=0xD4
op90  diff_bytes=119  size=0x1B0
op111 diff_bytes=109  size=0x1B4
op133 diff_bytes=103  size=0x18C
op93  diff_bytes=103  size=0x1C5
op94  diff_bytes=101  size=0x1DA
```

Only 39 of the 162 unique opcode spans in the shape report are byte-identical
under the temporary relocation replay.  The next phase is therefore broad
byte-level source-shape recovery, not a one-byte cleanup.

## op109 / vector-temp result

The old `op109 -3` extent issue is solved in the shape baseline.  The current
source emits the target hidden temporary slot:

```asm
lea edx, [ebp-0x128]
```

Do not revert `case 109` to a named `D3DXVECTOR3 sum` local; that returns the
handler to the short `[ebp-0x50]` local and reopens the original `-3` issue.

## op134 shape closure caveat

The shape-exact baseline uses the mathematical equivalence:

```cpp
(((flags >> 14) & 1) * ((flags >> 7) & 3)) == 0
```

The extracted values are small nonnegative masks, so this is semantically
equivalent to:

```cpp
((flags >> 14) & 1) == 0 || ((flags >> 7) & 3) == 0
```

However, the target machine code is a short-circuit OR.  The product form is a
shape-balancing tool, not byte-exact evidence for op134.  Replacing it with the
short-circuit macro currently reintroduces a one-byte span difference caused by
`mov edx,[abs]` versus target `mov eax,[abs]` on the second flags load.

## Tail / child-context experiments

Target tail has a real child-context local near the end of the frame:

```text
next         @ [ebp-0x100]
childContext @ [ebp-0x104]
```

Current shape-exact source has `next` but no stable `childContext` local and
reindexes `enemy+0x3384+next*4` repeatedly.  This explains the first strict
byte diff in the prologue (`0x5C0` instead of target `0x5C4`).

Natural C++ probes:

```text
all accesses through childContext:
  frame=0x5C4, op3=-36

childContext only used for the +0x228 store:
  frame=0x5C4, op3=+3

instruction and +0x220 through TH08_ECL_CURRENT_CONTEXT, store through current context:
  frame=0x5C4, op3=-2

instruction through current context, +0x228 through childContext:
  frame=0x5C4, op3=-5

childContext declared inside the if block:
  frame=0x5C4, op3=+12

remove inverse local without childContext:
  shape remains 0, but frame=0x5BC and byte replay worsens
```

The most promising strict-exact direction remains recovering a target-like
`childContext @ -0x104` while finding the real tail expression that preserves
`op3` extent and byte shape.  Do not preserve a childContext variant merely
because it fixes the prologue; it must also keep the tail span and byte replay
moving toward target.

## op2 register-shape probes

`case 2` is the first non-prologue byte-mismatching handler.  It is already
span-exact but has 13 byte mismatches caused by register allocation.  These
natural rewrites did **not** improve byte replay:

```cpp
context->secondaryTime.operator=(ReadInt(enemy, instruction, 0));
*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(context) + 0x90) =
    ReadInt(enemy, instruction, 0);
(void)(context->secondaryTime = ReadInt(enemy, instruction, 0));
```

Introducing an explicit `lhsInt`, `value`, or `ZunTimer *timer` local changes
span size and is not a useful byte-level fix for op2.

## Guardrails

- Do not use inline/native assembly, naked asm, fake byte arrays, arbitrary
  padding, or ABI lies to close byte gaps.
- Keep shape exact as a regression gate while doing byte probes.
- Use temporary manifests only under `.analysis/` until the byte replay is exact
  and relocation rows have been reviewed for canonical symbol ownership.

## Additional tail/op134 balancing probes

A `childContext` tail form with both `instruction` and the `+0x220` write routed
through `TH08_ECL_CURRENT_CONTEXT(unusedContext)` gives:

```text
frame=0x5C4
op3=-2
```

It looks target-like for the child-context data flow, but it cannot simply be
balanced with op134.  Pairing it with the smallest `fall_allowed` short-circuit
op134 variant gives total physical size zero, but byte replay gets much worse:

```text
base diff_count=5033
child/current-context + op134 fall_allowed diff_count=10879
```

Do not use op134 block reordering merely to offset tail size.  It preserves the
aggregate extent while destroying byte locality.

## VectorAngle tail call probe

The target position-restore tail calls the global helper at `0x0040C7B0`
(`th08::VectorAngle`) directly.  Current shape-exact source calls the provisional
`TargetApi::VectorAngle` thiscall wrapper.  A direct source rewrite to:

```cpp
VectorAngle(y, x)
```

builds and gives the desired global helper relocation, but by itself changes the
tail span:

```text
frame=0x5C0
op3=-10
```

So the direct global call is likely part of the eventual byte-exact form, but it
must be paired with other real tail-shape changes rather than applied alone.

## Opcode 127: basic-block order matters even when span is exact

Target opcode 127 executes the nonnegative slot-install path first and branches
to the negative cleanup path.  Writing the equivalent source with the cleanup
path first kept the handler extent exact but produced 324 relocated byte
mismatches.  Reordering the C++ to `if (ReadInt(...) >= 0) { install } else {
cleanup }` preserves the 0-delta span and reduces opcode 127 to 12 mismatching
bytes; full RunEcl relocated mismatch count drops from 5033 to 4721.

The remaining 12 bytes are four resolver scratch homes that are all four bytes
too shallow.  With the target-present 4-byte `childContext` tail local restored,
those homes move exactly to target offsets `-0x438/-0x43C/-0x440/-0x444`.
This is strong evidence that the missing frame dword is a shared cause rather
than an opcode-127-local expression problem.

## Opcode 87: a hidden float temporary is the RunEcl frame-size hinge

The decisive frame mismatch was not caused by the tail.  Target opcode 87
performs `if (enemyTable[ReadInt(2)])` directly, then assigns a conditional
float expression straight into `*WriteFloat(...)`.  This makes VC7 allocate a
hidden float result home at `[ebp-0x370]` after the two existing integer
resolver homes at `-0x368/-0x36C`.

The earlier reconstruction saved the first `ReadInt(2)` into shared `lhsInt`
and the float into shared `lhsFloat@-0x1C`.  That removed the hidden `-0x370`
dword.  As a consequence every compiler-owned resolver scratch allocated after
opcode 87 was four bytes too shallow, the function frame was `0x5C0` instead
of target `0x5C4`, and many otherwise-correct handlers differed only in stack
home bytes.

Target-faithful source shape:

```cpp
if (g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 2)])
    *WriteFloat(enemy, instruction, 0) =
        (instruction->operandFlags & 2U)
            ? g_EclEnemyTableF54CC0[ReadInt(enemy, instruction, 2)]
                  ->ResolveFloat(*reinterpret_cast<f32 *>(&RawInt(instruction, 1)))
            : *reinterpret_cast<f32 *>(&RawInt(instruction, 1));
```

This removes an extra six-byte `lhsInt` spill/reload, creates the target hidden
float home, keeps the opcode-87 extent exact, restores the `0x5C4` frame, and
moves all later resolver homes onto their target offsets.  In the current
RunEcl reconstruction it reduces relocated byte mismatches from 4721 to 4100;
opcodes 87, 88, 89, 127, 168, and 184 become byte-exact under replay.
