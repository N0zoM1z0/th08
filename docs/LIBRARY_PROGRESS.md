# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,403 / 216,403**
- Configured library match units: **78 functions / 10,339 body bytes**
- Exact library functions: **78 / 1,113 (7.01%)**
- Exact library body bytes: **10,339 / 216,403 (4.78%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
