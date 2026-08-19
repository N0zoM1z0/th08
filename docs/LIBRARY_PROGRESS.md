# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,588 / 216,588**
- Configured library match units: **120 functions / 15,917 body bytes**
- Exact library functions: **120 / 1,113 (10.78%)**
- Exact library body bytes: **15,917 / 216,588 (7.35%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
