# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,119 / 1,119**
- Inventory-mapped library bytes: **217,165 / 217,165**
- Configured library match units: **231 functions / 34,087 body bytes**
- Exact library functions: **231 / 1,119 (20.64%)**
- Exact library body bytes: **34,087 / 217,165 (15.70%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
