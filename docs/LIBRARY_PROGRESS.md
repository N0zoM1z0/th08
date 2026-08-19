# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,403 / 216,403**
- Configured library match units: **74 functions / 9,770 body bytes**
- Exact library functions: **74 / 1,113 (6.65%)**
- Exact library body bytes: **9,770 / 216,403 (4.51%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
