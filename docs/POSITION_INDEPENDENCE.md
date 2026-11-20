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
