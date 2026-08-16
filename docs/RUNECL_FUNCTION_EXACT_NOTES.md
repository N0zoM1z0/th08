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

## Opcodes 93/94: nested scope recovers packet/position local order

The target constructs `SpawnPacketTyped` first, then `D3DXVECTOR3 position`, and
only then copies the seven instruction operands into the packet.  Despite that
constructor order, target stack layout is packet-shallow / position-deep.  A
flat block makes VC7 allocate the two locals in the opposite physical order.

The matching source keeps `packet` and the `spawned` result in the outer block,
then introduces an inner block containing `position`; `memcpy` occurs after the
inner `position` declaration.  The first three resolved floats are read from
`packet.position`, not from the instruction again.  This produces:

- op93: packet `-0xA8`, position `-0xB4`, spawned `-0x8C`;
- op94: packet `-0xD4`, position `-0xE0`, spawned `-0xB8`;
- packet ctor -> position ctor -> `rep movsd`, exactly as target;
- integer-move raw branches for the three packet-local float fields.

With the opcode-87 frame fix already present, each handler becomes fully
byte-exact under relocation replay (op93 82 -> 0 mismatching bytes, op94 83 ->
0) while preserving the global zero shape score.

## Opcode 90: linked-child flag assignment is the first init statement

Target opcode 90 sets the new child's `linkedChild` bit immediately after the
initialization-enabled guard, before the three `IsYoukai`-driven updates.  The
earlier source placed this assignment after those updates.  Moving the same
bitfield assignment to the top of the guarded block keeps the handler extent
unchanged and reduces opcode-90 relocated mismatches from 119 to 24 (full
RunEcl strict replay improves by 95 bytes).  Opcodes 91/92 already had this
ordering.

The remaining opcode-90 residuals track the four-byte shallow-local shift from
the extra opcode-79 `flags` local; do not paper over them with register tricks.

## Opcode 114/115: mixed-width operand packet is not generic 4-byte indexing

The opcode-114 packet starts with a raw `u16` at `operands+0`, a conditional
signed `i16` at `+2`, then six floats at `+4,+8,+0xC,+0x10,+0x14,+0x18`, three
conditional ints at `+0x1C,+0x20,+0x24`, and three raw ints at
`+0x28,+0x2C,+0x30`.  Generic `TH08_ECL_READ_F_RAWARG(ctx, 2..7)` and
`TH08_ECL_READ_I(ctx, 8..10)` were therefore one dword late even though their
code lengths happened to fit the handler extent.

Spell these fields from the local `operands` pointer explicitly.  The
conditional float raw branch must use the same `f32` lvalue from `operands`, so
VC7 materializes the target hidden result home with an integer raw-branch copy.
With the current frame reconstruction this keeps opcode 114/115 span exact and
reduces its relocated byte mismatches from 148 to 44; full RunEcl strict replay
improves from 3840 to 3736 mismatching bytes.

## Opcode 119: conditional float raw branches must copy bits, not round-trip x87

For the three object-position deltas in opcode 119, target `ResolveFloat`
branches store the x87 result into compiler scratch, while the unresolved
branches copy the raw operand dword into the same scratch with integer moves.
`TH08_ECL_READ_F_RAWARG` still compiled the raw branch as `fld/fstp` in this
context.  Spelling each conditional explicitly with
`*reinterpret_cast<f32 *>(&TH08_ECL_RAW_I(...))` recovers the integer raw-copy
branch without changing the handler extent.

This reduces opcode-119 relocated mismatches from 70 to 30 and full RunEcl
strict replay from 3736 to 3696.  Reversing the commutative source addition did
not change VC7's x87 load order, so keep the natural existing `base + operand`
source order and only preserve the raw-bit conditional.

## Opcode 72: only operand 1 keeps the old raw-float source shape

Opcode 72 is not uniform across its six float operands.  Target first resolves
operand 0 once, stores it to `enemy+0x2DE8`, then assigns the same compiler
scratch to `ZunTimer@+0x2DDC`.  Operand 1 (`+0x2DD0`) retains the existing
`ReadFloatRawArg` x87 raw branch, while operands 2..6 use conditional
expressions whose unresolved branches copy raw dword bits into their hidden
float result homes with integer moves.

The target-faithful combination is therefore:

- direct chained timer assignment;
- leave operand 1 as `ReadFloatRawArg`;
- spell operands 2..6 as explicit `ResolveFloat(raw-bits) : raw-bits`;
- keep the final `flags |= 0x3000` source unchanged.

A bounded 128-combination probe found only two shape-zero variants before the
search timeout; this one reduces opcode-72 relocated mismatches from 350 to 26
and full RunEcl strict replay from 3696 to 3372 while preserving the global
zero shape score.  The remaining 26 bytes are a cyclic EAX/ECX/EDX register
phase, not a data-flow or stack-home mismatch.

## Opcodes 77/78 are the register-phase hinge for opcode 79

Opcodes 77 and 78 each assign two conditional floats.  Their unresolved
branches must copy the raw instruction dword into the compiler float result
home with integer moves.  Leaving either handler as `ReadFloatRawArg` produces
an x87 `fld/fstp` branch and also changes the register allocator phase entering
opcode 79.

Changing only opcode 77 or only opcode 78 makes the global handler extent grow
by four bytes.  Changing **both together** preserves the zero shape score and
makes both handlers byte-exact.  More importantly, the opcode-79 entry sequence
changes from `scratch -> EDX -> lhsInt -> EAX` to target
`scratch -> EAX -> lhsInt -> ECX`.

With that correct upstream phase, the natural opcode-79 reconstruction finally
matches without length tricks:

- remove the extra `flags` local and use shared `lhsInt` directly;
- assign the six observed one-bit fields through real bitfields;
- restore opcode 158's dedicated `index` local, which occupies target `-0x54`
  and balances the four-byte local removed from opcode 79.

Under the corrected 77/78 phase, opcode 79's bit-28 update naturally uses the
target ordinary-register `and ecx, 0xEFFFFFFF` rather than the one-byte-short
EAX encoding.  Opcodes 77, 78, 79, 80, 81, 90, 91, and 92 all become byte-exact
in the same candidate, while global shape remains zero.  On the current
RunEcl baseline, strict replay drops from 3372 to 2727 mismatching authored
bytes.

This is a reusable warning for giant VC7 functions: a seemingly local
`fld/fstp` versus integer raw-copy choice can rotate register allocation in the
next handler.  Fix upstream handlers before trying to force a downstream
register with source hacks.

## Opcode 66: re-evaluate full target flow only after upstream register-phase fixes

Opcode 66 should be evaluated as one control-flow unit: direct `ReadInt(0) <= 0`,
explicit raw-bit conditional floats for operands 2/3, movement-mode update,
timer reset, and an `else` call to `EclHelpers::ConfigurePolarMotion`.  Earlier
probes on the pre-op77/78 allocator phase were misleading because the whole
handler's EAX/ECX/EDX cycle was shifted.  The selector in `.analysis/select_op66_target.py`
compares this complete target-flow form against the current whole-function
strict replay and retains it only when both shape zero and byte improvement are
proven.
