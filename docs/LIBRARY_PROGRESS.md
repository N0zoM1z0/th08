# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,115 / 1,115**
- Inventory-mapped library bytes: **216,820 / 216,820**
- Configured library match units: **158 functions / 25,281 body bytes**
- Exact library functions: **158 / 1,115 (14.17%)**
- Exact library body bytes: **25,281 / 216,820 (11.66%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
