#!/usr/bin/env python3
"""Compare portable surface scaling with the pinned VC7 D3DX8 archive."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import tomllib

ROOT = Path(__file__).resolve().parents[2]
PROBE = ROOT / "tests/surface-scaling-probe.cpp"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def windows_path(path):
    return "Z:" + str(path.resolve()).replace("/", "\\")


def read_samples(path):
    text = path.read_text()
    start = text.find("[\n")
    if start < 0:
        raise ValueError("Scaling probe JSON is missing: " + str(path))
    rows, _ = json.JSONDecoder().raw_decode(text[start:])
    expected = [(f, s, p, k) for f in range(2) for s in range(4)
                for p in range(4) for k in range(1, 6)]
    actual = [(r["format"], r["shape"], r["pattern"], r["filter"]) for r in rows]
    if actual != expected:
        raise ValueError("Scaling probe case coverage differs")
    for row in rows:
        count = (64, 16, 64, 12)[row["shape"]]
        maximum = 65535 if row["format"] == 0 else 0xffffffff
        if len(row["pixels"]) != count or any(not 0 <= p <= maximum for p in row["pixels"]):
            raise ValueError("Scaling probe pixel coverage differs")
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True, help="Fresh directory under build/")
    parser.add_argument("--expect-filter-failure", action="store_true",
                        help="Require the current nearest-neighbor failure control")
    parser.add_argument("--check", action="store_true", help="Require all reference surfaces to match")
    args = parser.parse_args()
    if args.check and args.expect_filter_failure:
        parser.error("Choose --check or --expect-filter-failure")
    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / "build"):
        parser.error("Output must be a fresh directory under build/")
    for name in ("wine", "wineserver", "xvfb-run", "g++", "pkg-config"):
        if not shutil.which(name):
            parser.error("Required command is missing: " + name)
    manifest = tomllib.loads((ROOT / "config/library-provenance.toml").read_text())
    archive_info = next(a for a in manifest["archives"] if a["id"] == "vc7-d3dx8-prerelease")
    archive = ROOT / archive_info["path"]
    if sha(archive) != archive_info["sha256"]:
        parser.error("Pinned D3DX8 archive hash differs")
    subprocess.run(["python3", str(ROOT / "scripts/verify-target.py")], check=True)
    output.mkdir(parents=True)
    prefix = Path(tempfile.mkdtemp(prefix="surface-scaling-wine.", dir=ROOT / "build"))
    (prefix / ".th08-replay-owned").write_text("Owned surface scaling probe prefix\n")
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy", LIBGL_ALWAYS_SOFTWARE="1",
                       GALLIUM_DRIVER="llvmpipe", LP_NUM_THREADS="0")
    environment.pop("LP_NO_RAST", None)
    native_env = dict(environment, WINEPREFIX=str(prefix), WINEARCH="win32", WINEDEBUG="-all",
                      WINEDLLOVERRIDES="winemenubuilder.exe=d;winepulse.drv,winealsa.drv=d")
    native = output / "scaling-probe.exe"
    obj = output / "scaling-probe.obj"
    batch = output / "build-native.bat"
    batch.write_text(
        '@echo off\ncall "' + windows_path(ROOT / "scripts/th08vars.bat") + '"\n'
        'cl /nologo /O2 /MT /EHsc "' + windows_path(PROBE) + '" '
        '"/Fo' + windows_path(obj) + '" "/Fe' + windows_path(native) + '" '
        '/link "/MAP:' + windows_path(output / "native.map") + '" '
        '"' + windows_path(archive) + '" d3d8.lib user32.lib gdi32.lib advapi32.lib\n'
        'exit /b %errorlevel%\n')
    observations = {}
    binary_hashes = {}
    try:
        print("Building VC7 D3DX8 reference probe", flush=True)
        with (output / "native-build.log").open("w") as log:
            subprocess.run(["wine", "cmd", "/c", windows_path(batch)], cwd=output,
                           env=native_env, stdout=log, stderr=subprocess.STDOUT, timeout=120, check=True)
        binary_hashes["native"] = sha(native)
        print("Capturing pinned D3DX8 reference surfaces", flush=True)
        with (output / "native.stdout").open("w") as stdout, (output / "native.log").open("w") as stderr:
            subprocess.run(["xvfb-run", "-a", "wine", str(native)], cwd=output, env=native_env,
                           stdout=stdout, stderr=stderr, timeout=90, check=True)
        observations["native"] = read_samples(output / "native.stdout")
    finally:
        subprocess.run(["wineserver", "-k"], env=native_env, timeout=15, check=False)
        subprocess.run(["wineserver", "-w"], env=native_env, timeout=15, check=False)
        if not (prefix / ".th08-replay-owned").is_file():
            raise RuntimeError("Owned scaling prefix marker is missing")
        shutil.rmtree(prefix)
    environment["PKG_CONFIG_LIBDIR"] = "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig"
    flags = shlex.split(subprocess.check_output(
        ["pkg-config", "--cflags", "--libs", "sdl2", "SDL2_image", "SDL2_ttf", "fontconfig", "gl"],
        env=environment, text=True))
    portable = output / "scaling-probe-portable"
    sources = [ROOT / ("src/modern/linux/" + name) for name in
               ("d3d8_compat.cpp", "d3dx8_compat.cpp", "linux_compat.cpp")]
    command = ["g++", "-std=gnu++98", "-O2", "-fno-strict-aliasing", "-pthread",
               "-DTH08_MODERN_PORT", "-DTH08_MODERN_LINUX", "-DTH08_PORTABLE_NATIVE_LAYOUT",
               "-D_FILE_OFFSET_BITS=64", "-D_TIME_BITS=64", "-Isrc", "-Isrc/modern/linux/include",
               "-include", "src/modern/linux/linux_compat.hpp", "-Isrc/modern/linux", str(PROBE),
               *map(str, sources), "-o", str(portable), *flags, "-ldl"]
    print("Building portable surface probe", flush=True)
    with (output / "portable-build.log").open("w") as log:
        subprocess.run(command, cwd=ROOT, env=environment, stdout=log, stderr=subprocess.STDOUT,
                       timeout=120, check=True)
    binary_hashes["portable"] = sha(portable)
    with (output / "portable.stdout").open("w") as stdout:
        subprocess.run(["xvfb-run", "-a", str(portable)], env=environment, stdout=stdout,
                       stderr=subprocess.STDOUT, timeout=45, check=True)
    observations["portable"] = read_samples(output / "portable.stdout")
    comparisons = []
    for reference, candidate in zip(observations["native"], observations["portable"]):
        different = sum(a != b for a, b in zip(reference["pixels"], candidate["pixels"]))
        comparisons.append(dict(format=reference["format"], shape=reference["shape"],
                                pattern=reference["pattern"], filter=reference["filter"],
                                referenceHresult=reference["hresult"], candidateHresult=candidate["hresult"],
                                differentPixels=different, pixels=len(reference["pixels"]),
                                equal=not different and reference["hresult"] == candidate["hresult"]))
    for name, rows in observations.items():
        (output / (name + ".json")).write_text(json.dumps(rows, indent=2) + "\n")
    filter_sensitive = {}
    for name, rows in observations.items():
        groups = {}
        for row in rows:
            groups.setdefault((row["format"], row["shape"], row["pattern"]), []).append(row)
        filter_sensitive[name] = sum(len({(r["hresult"], tuple(r["pixels"])) for r in group}) > 1
                                     for group in groups.values())
    vc7_bin = ROOT / "scripts/prefix/PROGRAM FILES/MICROSOFT VISUAL STUDIO .NET/VC7/BIN"
    report = dict(evidenceClass="Pinned VC7 D3DX8 archive API control; not execution inside the retail game",
                  targetSha256=sha(ROOT / "resources/th08.exe"), archiveSha256=sha(archive),
                  probeSha256=sha(PROBE), runnerSha256=sha(Path(__file__)), binarySha256=binary_hashes,
                  nativeCompilerSha256={name: sha(vc7_bin / name) for name in ("CL.EXE", "C1XX.DLL", "C2.DLL", "LINK.EXE")},
                  nativeMapSha256=sha(output / "native.map"),
                  sourceSha256={str(p.relative_to(ROOT)): sha(p) for p in sources},
                  sourceCommit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                  portableCompiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
                  wineVersion=subprocess.check_output(["wine", "--version"], text=True).strip(),
                  samples=len(comparisons), exactSamples=sum(c["equal"] for c in comparisons),
                  filterSensitiveGroups=filter_sensitive, filterGroups=32,
                  ownedPrefixRemoved=True, comparisons=comparisons)
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    for path in (native, obj, portable):
        path.unlink()
    for path in output.glob("*.log"):
        with gzip.open(str(path) + ".gz", "wb") as destination:
            destination.write(path.read_bytes())
        path.unlink()
    print("Surface scaling: %d/%d exact samples; see %s" %
          (report["exactSamples"], report["samples"], output / "report.json"))
    if args.check and report["exactSamples"] != report["samples"]:
        raise SystemExit("Surface scaling differs from the pinned D3DX8 reference")
    if args.expect_filter_failure:
        equal_size = [c for c in comparisons if c["shape"] == 0]
        point_scaled = [c for c in comparisons if c["shape"] != 0 and c["filter"] == 2]
        title_scaled = [c for c in comparisons if c["shape"] == 1 and c["filter"] == 4]
        if (not all(c["equal"] for c in equal_size + point_scaled) or
                any(c["equal"] or c["referenceHresult"] != 0 or c["candidateHresult"] != 0
                    for c in title_scaled) or filter_sensitive != dict(native=24, portable=0)):
            raise SystemExit("Nearest-neighbor failure control was not reproduced")
        print("PASS: equal-size and point copies match; all eight title-scale cases expose ignored filtering")


if __name__ == "__main__":
    main()
