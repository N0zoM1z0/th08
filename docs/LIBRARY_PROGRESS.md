# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,327 / 216,327**
- Configured library match units: **63 functions / 8,387 body bytes**
- Exact library functions: **63 / 1,112 (5.67%)**
- Exact library body bytes: **8,387 / 216,327 (3.88%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
