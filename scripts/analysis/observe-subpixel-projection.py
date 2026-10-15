"""GDB helper for the opt-in subpixel probe; uses hardware breakpoints only."""
import hashlib
import json
import os
from pathlib import Path
import struct

import gdb

# Wine 9.0 Ubuntu i386 build. These offsets describe Wine, not the TH08 target.
WINE_SHA = "cd50dcd7ac2355f8ce6b429eaaf1e828ac1df656cc9846cce381122c41917ab4"
DLL = Path("/usr/lib/i386-linux-gnu/wine/i386-windows/wined3d.dll")
PROJECTION_RVA = 0x106CC0
ENTRY_BYTES = bytes.fromhex("5589e557565383ec548b7d088b750c8b")


def observe():
    if hashlib.sha256(DLL.read_bytes()).hexdigest() != WINE_SHA:
        raise RuntimeError("Unsupported Wine DLL: projection observer requires the pinned i386 build")
    output = Path(os.environ["TH08_SUBPIXEL_OBSERVER_OUTPUT"])
    for command in ("set pagination off", "set confirm off", "set debuginfod enabled off",
                    "set disable-randomization off",
                    "handle SIGUSR1 SIGUSR2 SIGSEGV SIGFPE nostop noprint pass",
                    "handle SIGTRAP stop print nopass"):
        gdb.execute(command)
    gdb.execute("run")
    base = int((output / "wined3d-module.txt").read_text().strip(), 16)
    address = base + PROJECTION_RVA
    actual = bytes(gdb.selected_inferior().read_memory(address, len(ENTRY_BYTES)))
    if actual != ENTRY_BYTES:
        raise RuntimeError("Mapped Wine projection entry differs from the pinned DLL")
    records, pending, returns = [], {}, {}
    report = dict(evidenceClass="Observed Wine D3D8 synthetic-probe projection matrices",
                  dll=str(DLL), dllSha256=WINE_SHA, functionRva=PROJECTION_RVA,
                  moduleBase=base, entryBytes=actual.hex(), records=records)

    class Returned(gdb.Breakpoint):
        def __init__(self, return_address):
            super().__init__("*" + hex(return_address), type=gdb.BP_HARDWARE_BREAKPOINT, internal=True)

        def stop(self):
            key = gdb.selected_thread().ptid
            if key not in pending:
                return False
            pointer = pending.pop(key)
            data = bytes(gdb.selected_inferior().read_memory(pointer, 64))
            records.append(dict(thread=list(key), words=list(struct.unpack("<16I", data)),
                                floats=list(struct.unpack("<16f", data))))
            if len(records) > 256:
                raise RuntimeError("Projection observation limit exceeded")
            return False

    class Entered(gdb.Breakpoint):
        def stop(self):
            stack = int(gdb.parse_and_eval("$esp"))
            data = bytes(gdb.selected_inferior().read_memory(stack, 16))
            ret, context, state, matrix = struct.unpack("<4I", data)
            if not all((ret, context, state, matrix)) or len(returns) > 2:
                raise RuntimeError("Unexpected Wine projection call")
            if ret not in returns:
                returns[ret] = Returned(ret)
            key = gdb.selected_thread().ptid
            if key in pending:
                raise RuntimeError("Unexpected recursive projection call")
            pending[key] = matrix
            return False

    entry = Entered("*" + hex(address), type=gdb.BP_HARDWARE_BREAKPOINT, internal=True)
    gdb.execute("continue")
    if int(gdb.parse_and_eval("$_exitcode")) != 0 or pending or not records:
        raise RuntimeError("Projection probe did not complete")
    (output / "projection.json").write_text(json.dumps(report, indent=2) + "\n")
    entry.delete()


try:
    observe()
except Exception as error:
    print("Projection observation failed: " + str(error))
    gdb.execute("quit 1")
