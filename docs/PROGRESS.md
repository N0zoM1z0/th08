# Reconstruction progress

Generated from `config/reccmp-functions.csv`, `config/mapping.csv`,
`config/implemented.csv`, and the separate exact ledgers.

> `implemented.csv` records source presence only. Exact figures below
> count only reproducible 100% comparisons tracked in `config/matches.csv`.

- Source-present authored functions: **1,107 / 1,107 (100.00%)**
- Source-present authored bytes: **459,757 / 459,757 (100.00%)**
- Exact authored functions: **1,091 / 1,107 (98.55%)**
- Exact authored bytes: **445,728 / 459,757 (96.95%)**
- Inventory-classified library functions: **1,115**
- Exact library functions: **163 / 1,115 (14.62%)**
- Exact library bytes: **26,428 / 216,820 (12.19%)**

The public SVG visualizes exact bytes only;
source presence never fills the progress bar.

Function sizes and library classification are imported analysis seeds and
must be reconciled against the hash-attested target before exact acceptance.
