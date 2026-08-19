# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,403 / 216,403**
- Configured library match units: **76 functions / 9,820 body bytes**
- Exact library functions: **76 / 1,113 (6.83%)**
- Exact library body bytes: **9,820 / 216,403 (4.54%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
