# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,256 / 216,256**
- Configured library match units: **7 functions / 1,145 body bytes**
- Exact library functions: **0 / 1,112 (0.00%)**
- Exact library body bytes: **0 / 216,256 (0.00%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
