# Reconstruction progress

Generated from `config/reccmp-functions.csv`, `config/mapping.csv`,
`config/implemented.csv`, and the separate exact ledgers.

> `implemented.csv` records source presence only. Exact figures below
> count only reproducible 100% comparisons tracked in `config/matches.csv`.

- Source-present authored functions: **1,107 / 1,107 (100.00%)**
- Source-present authored bytes: **459,757 / 459,757 (100.00%)**
- Exact authored functions: **1,105 / 1,107 (99.82%)**
- Exact authored bytes: **459,115 / 459,757 (99.86%)**
- Inventory-classified library functions: **1,119**
- Exact library functions: **258 / 1,119 (23.06%)**
- Exact library bytes: **52,707 / 217,165 (24.27%)**

The public SVG visualizes exact bytes only;
source presence never fills the progress bar.

Function sizes and library classification are imported analysis seeds and
must be reconciled against the hash-attested target before exact acceptance.
