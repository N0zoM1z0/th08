# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,288 / 216,288**
- Configured library match units: **50 functions / 7,258 body bytes**
- Exact library functions: **50 / 1,112 (4.50%)**
- Exact library body bytes: **7,258 / 216,288 (3.36%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
