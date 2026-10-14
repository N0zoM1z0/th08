#!/usr/bin/env python3
"""Compare Wine and portable text blending, clipping, overlap and unused bitmap bits."""
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

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def records(path):
    raw = path.read_text()
    rows, _ = json.JSONDecoder().raw_decode(raw[raw.index('[\n'):])
    if [row['id'] for row in rows] != list(range(96)):
        raise ValueError('Font blend probe must contain all 96 cases')
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--font', type=Path, required=True)
    parser.add_argument('--coverage-reference', type=Path, required=True, help='Completed probe-freetype-architecture.py output')
    parser.add_argument('--baseline-ref', help='Also check the adapter at an immutable Git commit')
    parser.add_argument('--weight', choices=('regular', 'semibold'), default='semibold')
    parser.add_argument('--output-dir', type=Path, required=True, help='Fresh directory under build/')
    args = parser.parse_args()
    output, font = args.output_dir.resolve(), args.font.resolve(strict=True)
    if output.exists() or not output.is_relative_to(ROOT / 'build'):
        parser.error('Output must be a fresh directory under build/')
    for command in ('g++', 'i686-w64-mingw32-g++', 'pkg-config', 'wine', 'wineserver', 'xvfb-run', 'timeout'):
        if not shutil.which(command):
            parser.error('Required command is missing: ' + command)
    coverage = json.loads((args.coverage_reference / 'comparison.json').read_text())
    glyph = json.loads((args.coverage_reference / 'i386.json').read_text())
    if coverage['fontSha256'] != digest(font) or coverage['codepoint'] != 0x90F7 or coverage['ppem'] != 21:
        parser.error('Coverage control font, glyph or size differs')
    if not coverage['outlineBeforeEqual'] or not coverage['outlineAfterEqual']:
        parser.error('Coverage control must have equal outlines')
    weight = 400 if args.weight == 'regular' else 600
    if coverage['strength26dot6'] != (0 if weight == 400 else 56):
        parser.error('Coverage control bold strength differs from the selected weight')
    observed = [(p['x'], p['y']) for p in coverage['bitmapDifferences'] if p['i386Level'] != p['x86_64Level']]
    ramp = (0, 77, 104, 124, 140, 154, 167, 178, 189, 199, 208, 217, 225, 233, 240, 248, 255)
    # Colored text uses both the forward and reverse intensity ranges.
    channel_bound = max((max(abs(ramp[p['i386Level']] - ramp[p['x86_64Level']]),
                             abs(ramp[16 - p['i386Level']] - ramp[16 - p['x86_64Level']])) + 1
                         for p in coverage['bitmapDifferences'] if p['i386Level'] != p['x86_64Level']), default=0)
    output.mkdir(parents=True)
    source = ROOT / 'tests/font-blend-probe.cpp'
    native = output / 'native'
    native.mkdir()
    binary = output / 'font-blend.exe'
    with (output / 'native-build.log').open('w') as log:
        subprocess.run(['i686-w64-mingw32-g++', '-O2', '-static', '-DTH08_FONT_PROBE_WEIGHT=' + str(weight),
                        str(source), '-o', str(binary), '-lgdi32', '-luser32'],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    prefix = Path(tempfile.mkdtemp(prefix='font-blending-wine.', dir=ROOT / 'build'))
    (prefix / '.th08-replay-owned').write_text('Owned font blending probe prefix\n')
    environment = dict(os.environ, TH08_FONT=str(font), SDL_AUDIODRIVER='dummy', WINEPREFIX=str(prefix),
                       WINEARCH='win32', WINEDEBUG='-all', WINEDLLOVERRIDES='winepulse.drv,winealsa.drv=d')
    try:
        with (native / 'stdout.txt').open('w') as stdout, (native / 'wine.log').open('w') as log:
            subprocess.run(['xvfb-run', '-a', 'timeout', '--kill-after=5s', '60s', 'wine', str(binary),
                            'Z:' + native.as_posix()], env=environment, stdout=stdout, stderr=log, check=True)
    finally:
        subprocess.run(['wineserver', '-k'], env=environment, timeout=15, check=False)
        subprocess.run(['wineserver', '-w'], env=environment, timeout=15, check=False)
        shutil.rmtree(prefix)
        binary.unlink()
    reference = records(native / 'stdout.txt')
    flags = shlex.split(subprocess.check_output(['pkg-config', '--cflags', '--libs', 'sdl2', 'SDL2_image',
                                                 'SDL2_ttf', 'fontconfig', 'freetype2', 'gl'], text=True))
    adapters = {'production': ROOT / 'src/modern/linux/linux_compat.cpp'}
    if args.baseline_ref:
        commit = subprocess.check_output(['git', 'rev-parse', '--verify', '--end-of-options', args.baseline_ref + '^{commit}'], text=True).strip()
        adapter = output / 'baseline.cpp'
        adapter.write_bytes(subprocess.check_output(['git', 'show', commit + ':src/modern/linux/linux_compat.cpp']))
        adapters['baseline'] = adapter
    report = dict(evidenceClass='Wine GDI and portable API captures; no TH08 execution',
                  fontSha256=digest(font), coverageComparisonSha256=digest(args.coverage_reference / 'comparison.json'),
                  probeSha256=digest(source), runnerSha256=digest(Path(__file__)),
                  maximumMeasuredCoverageChannelError=channel_bound, weight=weight, variants={})
    failed = False
    for name, adapter in adapters.items():
        folder = output / name
        folder.mkdir()
        binary = output / ('font-blend-' + name)
        command = ['g++', '-std=gnu++98', '-O2', '-fno-strict-aliasing', '-pthread', '-DTH08_MODERN_PORT',
                   '-DTH08_FONT_PROBE_WEIGHT=' + str(weight),
                   '-DTH08_MODERN_LINUX', '-DTH08_PORTABLE_NATIVE_LAYOUT', '-D_FILE_OFFSET_BITS=64', '-D_TIME_BITS=64',
                   '-Isrc', '-Isrc/modern/linux/include', '-include', 'src/modern/linux/linux_compat.hpp',
                   '-Isrc/modern/linux', str(source), str(adapter), 'src/modern/linux/d3d8_compat.cpp',
                   'src/modern/linux/d3dx8_compat.cpp', '-o', str(binary), *flags, '-ldl']
        try:
            with (folder / 'build.log').open('w') as log:
                subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
            with (folder / 'stdout.txt').open('w') as stdout:
                subprocess.run(['timeout', '30s', str(binary), str(folder)], env=environment, stdout=stdout, stderr=subprocess.STDOUT, check=True)
        finally:
            binary.unlink(missing_ok=True)
        candidate = records(folder / 'stdout.txt')
        comparisons = []
        for ref, got in zip(reference, candidate):
            if any(ref[k] != got[k] for k in ('id', 'bits', 'background', 'foreground', 'mode')):
                raise ValueError('Font blending case identity differs')
            filename = f"blend-{ref['id']}.bin"
            left, right = (p.read_bytes() for p in (native / filename, folder / filename))
            step = ref['bits'] // 8
            if len(left) != 64 * 64 * step or len(right) != len(left):
                raise ValueError('Bitmap byte count differs')
            draws = [(-6, -8)] if ref['mode'] == 1 else [(4, 4)]
            if ref['mode'] == 2:
                draws.append((8, 8))
            known = {(x + glyph['bitmapLeft'] + px, y + ref['ascent'] - glyph['bitmapTop'] + py)
                     for x, y in draws for px, py in observed}
            differences = []
            for pixel in range(64 * 64):
                offset = pixel * step
                if left[offset:offset + step] != right[offset:offset + step]:
                    xy = pixel % 64, pixel // 64
                    a, b = left[offset:offset + step], right[offset:offset + step]
                    if step == 4:
                        bounded = a[3] == b[3] and max(abs(a[k] - b[k]) for k in range(3)) <= channel_bound
                    else:
                        pa, pb = int.from_bytes(a, 'little'), int.from_bytes(b, 'little')
                        bounded = (pa ^ pb) & 0x8000 == 0 and max(abs(((pa >> k) & 31) - ((pb >> k) & 31))
                                                                 for k in (0, 5, 10)) <= (channel_bound + 7) // 8
                    differences.append(dict(x=xy[0], y=xy[1], reference=left[offset:offset + step].hex(),
                                            candidate=right[offset:offset + step].hex(), measuredCoverageLocation=xy in known,
                                            boundedCoverageDifference=xy in known and bounded))
            unexplained = sum(not p['boundedCoverageDifference'] for p in differences)
            comparisons.append(dict(case=ref, differingPixels=len(differences), unexplainedPixels=unexplained,
                                    referenceSha256=digest(native / filename), candidateSha256=digest(folder / filename), differences=differences))
        unexplained = sum(r['unexplainedPixels'] for r in comparisons)
        report['variants'][name] = dict(adapterSha256=digest(adapter), cases=comparisons,
                                       exactCases=sum(not r['differingPixels'] for r in comparisons), unexplainedPixels=unexplained)
        print(f"{name}: 96 cases; {report['variants'][name]['exactCases']} pixel-identical; {unexplained} unexplained pixels")
        failed |= name == 'production' and unexplained != 0
    (output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    for path in output.rglob('*.bin'):
        with gzip.open(str(path) + '.gz', 'wb', compresslevel=9) as compressed:
            compressed.write(path.read_bytes())
        path.unlink()
    if failed:
        raise SystemExit('Production font blending differs outside measured FreeType coverage locations')


if __name__ == '__main__':
    main()
