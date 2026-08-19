# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,113 / 1,113**
- Inventory-mapped library bytes: **216,641 / 216,641**
- Configured library match units: **134 functions / 18,708 body bytes**
- Exact library functions: **134 / 1,113 (12.04%)**
- Exact library body bytes: **18,708 / 216,641 (8.64%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
