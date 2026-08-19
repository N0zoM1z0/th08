# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,114 / 1,114**
- Inventory-mapped library bytes: **216,656 / 216,656**
- Configured library match units: **145 functions / 19,442 body bytes**
- Exact library functions: **145 / 1,114 (13.02%)**
- Exact library body bytes: **19,442 / 216,656 (8.97%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
