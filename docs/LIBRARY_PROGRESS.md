# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,112 / 1,112**
- Inventory-mapped library bytes: **216,347 / 216,347**
- Configured library match units: **68 functions / 9,178 body bytes**
- Exact library functions: **68 / 1,112 (6.12%)**
- Exact library body bytes: **9,178 / 216,347 (4.24%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
