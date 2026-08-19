# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,116 / 1,116**
- Inventory-mapped library bytes: **216,923 / 216,923**
- Configured library match units: **203 functions / 32,003 body bytes**
- Exact library functions: **203 / 1,116 (18.19%)**
- Exact library body bytes: **32,003 / 216,923 (14.75%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
