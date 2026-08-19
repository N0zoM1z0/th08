# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,588 / 216,588**
- Configured library match units: **93 functions / 12,329 body bytes**
- Exact library functions: **93 / 1,113 (8.36%)**
- Exact library body bytes: **12,329 / 216,588 (5.69%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
