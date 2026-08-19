# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,560 / 216,560**
- Configured library match units: **83 functions / 11,277 body bytes**
- Exact library functions: **83 / 1,113 (7.46%)**
- Exact library body bytes: **11,277 / 216,560 (5.21%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
