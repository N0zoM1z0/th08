#!/usr/bin/env python3
"""Measure VC7 anonymous-namespace names for identical source in two paths."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from coff import ObjectModule
from winhelpers import run_windows_program

SOURCE = b"namespace th08 { namespace { int Helper(int value) { return value + 1; } } int Dispatch(int value) { return Helper(value); } }\n"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=Path("build/anonymous-namespace-control"))
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(ROOT / "build") or output.exists():
        parser.error("Output must be a fresh directory under build/")
    rows = []
    for variant in ("a", "b"):
        folder = output / variant
        folder.mkdir(parents=True)
        source, obj = folder / "probe.cpp", folder / "probe.obj"
        source.write_bytes(SOURCE)
        relative = lambda path: str(path.relative_to(ROOT)).replace("/", "\\")
        # Keep output/PDB paths fixed so only the source path changes.
        compiled = output / "compiled.obj"
        run_windows_program([str(ROOT / "scripts/th08run.bat"), "cl.exe", "/nologo", "/c", "/Gy", "/Gr", "/MT", "/Zi",
                             relative(source), "/Fo" + relative(compiled), "/Fd" + relative(output / "compiler.pdb")], cwd=str(ROOT))
        shutil.copyfile(compiled, obj)
        module = ObjectModule()
        module.unpack(obj.read_bytes())
        symbols = [(symbol, symbol.get_name(module.string_table).decode("ascii")) for symbol in module.symbols]
        helpers = [(symbol, name) for symbol, name in symbols if name.startswith("?Helper@?A0x") and symbol.section_number > 0]
        if len(helpers) != 1:
            raise ValueError("Expected one section-defined anonymous Helper symbol")
        symbol, name = helpers[0]
        size = symbol.aux_records[0].total_size
        code = module.sections[symbol.section_number - 1].data[symbol.value:symbol.value + size]
        rows.append(dict(sourcePath=relative(source), sourceSha256=sha(source.read_bytes()),
                         objectSha256=sha(obj.read_bytes()), helperSymbol=name,
                         helperSize=size, helperCodeSha256=sha(code)))
    result = (rows[0]["helperSymbol"] != rows[1]["helperSymbol"]
              and rows[0]["helperCodeSha256"] == rows[1]["helperCodeSha256"])
    report = dict(result="observed" if result else "unexpected", identicalSourceBytes=True,
                  scope="VC7 compiler-name control; no TH08 target comparison", records=rows)
    (output / "result.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))
    return 0 if result else 1


if __name__ == "__main__":
    raise SystemExit(main())
