# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,116 / 1,116**
- Inventory-mapped library bytes: **216,923 / 216,923**
- Configured library match units: **209 functions / 32,586 body bytes**
- Exact library functions: **209 / 1,116 (18.73%)**
- Exact library body bytes: **32,586 / 216,923 (15.02%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
