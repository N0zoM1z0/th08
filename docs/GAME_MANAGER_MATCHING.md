# GameManager matching notes

- `FUN_0043BBE1 @ 0x0043BBE1` proves that VC7 `/Os /Ob1` distinguishes a native array-element increment from decompiler-style assignment. The practice counter must be written as `pscrData[shot].attempts[stage][difficulty]++`; spelling it as `a = a + 1` reverses the stage/shot index materialization, grows the function from 0x228 to 0x229, and shifts the remaining block by one byte.
