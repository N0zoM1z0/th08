# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,115 / 1,115**
- Inventory-mapped library bytes: **216,820 / 216,820**
- Configured library match units: **163 functions / 26,428 body bytes**
- Exact library functions: **163 / 1,115 (14.62%)**
- Exact library body bytes: **26,428 / 216,820 (12.19%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
