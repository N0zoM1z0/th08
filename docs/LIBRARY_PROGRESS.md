# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,115 / 1,115**
- Inventory-mapped library bytes: **216,731 / 216,731**
- Configured library match units: **147 functions / 19,578 body bytes**
- Exact library functions: **147 / 1,115 (13.18%)**
- Exact library body bytes: **19,578 / 216,731 (9.03%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
