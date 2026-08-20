# GameManager matching notes

- `FUN_0043BBE1 @ 0x0043BBE1` proves that VC7 `/Os /Ob1` distinguishes a native array-element increment from decompiler-style assignment. The practice counter must be written as `pscrData[shot].attempts[stage][difficulty]++`; spelling it as `a = a + 1` reverses the stage/shot index materialization, grows the function from 0x228 to 0x229, and shifts the remaining block by one byte.

## Production ownership and order

The class name is not the production-object contract. Current exact owners are:

- early setters `SetLives`, `UpdateAntiTamper`, `SetBombCount`, and `SetPower`:
  `AsciiManager.obj`;
- `IsSoloHuman`, `IsSoloYoukai`, and `GetLives`: `EnemyManager.obj`;
- `ScaleFloatBasedOnRank`: the target-contiguous
  `AsciiManagerBossMarker.obj` block at `0x00422B80..0x00422C13`;
- `RandomizeAntiTamper`, `AddToDeaths`, and `AddToBombsUsed`: the explicit
  Player-local block at `0x0044E160..0x0044E348`;
- the continuous `0x00439829..0x0043C6F5` main region, including explicit
  `AddLives` between the constructor and arcade initializer: `GameManager.obj`.

These owners are backed by mapped target neighborhoods, production callers,
and strict replay, not just byte-identical duplicate sections. See the combined
case study in `KNOWLEDGE_BASE.md` before changing header visibility, PCH include
order, or an owner in `match-units.toml`.
