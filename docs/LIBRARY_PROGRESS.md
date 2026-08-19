# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,589 / 216,589**
- Configured library match units: **127 functions / 16,922 body bytes**
- Exact library functions: **127 / 1,113 (11.41%)**
- Exact library body bytes: **16,922 / 216,589 (7.81%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
