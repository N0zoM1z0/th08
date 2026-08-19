# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,263 / 216,263**
- Configured library match units: **48 functions / 6,603 body bytes**
- Exact library functions: **48 / 1,112 (4.32%)**
- Exact library body bytes: **6,603 / 216,263 (3.05%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
