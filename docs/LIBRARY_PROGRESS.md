# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,567 / 216,567**
- Configured library match units: **86 functions / 11,457 body bytes**
- Exact library functions: **86 / 1,113 (7.73%)**
- Exact library body bytes: **11,457 / 216,567 (5.29%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
