# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,115 / 1,115**
- Inventory-mapped library bytes: **216,820 / 216,820**
- Configured library match units: **172 functions / 27,280 body bytes**
- Exact library functions: **172 / 1,115 (15.43%)**
- Exact library body bytes: **27,280 / 216,820 (12.58%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
