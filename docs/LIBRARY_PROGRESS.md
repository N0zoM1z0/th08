# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,115 / 1,115**
- Inventory-mapped library bytes: **216,820 / 216,820**
- Configured library match units: **186 functions / 29,665 body bytes**
- Exact library functions: **186 / 1,115 (16.68%)**
- Exact library body bytes: **29,665 / 216,820 (13.68%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
