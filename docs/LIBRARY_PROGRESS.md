# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,327 / 216,327**
- Configured library match units: **60 functions / 7,939 body bytes**
- Exact library functions: **60 / 1,112 (5.40%)**
- Exact library body bytes: **7,939 / 216,327 (3.67%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
