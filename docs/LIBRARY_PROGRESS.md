# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,288 / 216,288**
- Configured library match units: **49 functions / 6,776 body bytes**
- Exact library functions: **49 / 1,112 (4.41%)**
- Exact library body bytes: **6,776 / 216,288 (3.13%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
