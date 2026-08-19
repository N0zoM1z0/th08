# Library/runtime reconstruction progress

Generated independently from the authored progress ledger. Library exactness
counts only rows accepted in `config/library-matches.csv` after archive-hash,
COFF relocation, extent, and canonical-target replay.

- Inventory-mapped library functions: **1,119 / 1,119**
- Inventory-mapped library bytes: **217,165 / 217,165**
- Configured library match units: **240 functions / 37,373 body bytes**
- Exact library functions: **240 / 1,119 (21.45%)**
- Exact library body bytes: **37,373 / 217,165 (17.21%)**

Archive-member padding and compiler-owned associated bytes may be included in
`compare_size`, but they do not inflate library body-byte progress.
