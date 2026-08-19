# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,115 / 1,115**
- Inventory-mapped library bytes: **216,787 / 216,787**
- Configured library match units: **150 functions / 24,496 body bytes**
- Exact library functions: **150 / 1,115 (13.45%)**
- Exact library body bytes: **24,496 / 216,787 (11.30%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
