# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,288 / 216,288**
- Configured library match units: **52 functions / 7,399 body bytes**
- Exact library functions: **52 / 1,112 (4.68%)**
- Exact library body bytes: **7,399 / 216,288 (3.42%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
