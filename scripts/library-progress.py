#!/usr/bin/env python3
"""Generate progress for target-linked library/runtime reconstruction only.

Examples:
  python3 scripts/library-progress.py
  python3 scripts/library-progress.py --check
"""
from __future__ import annotations
import csv
from pathlib import Path
import sys
import tomllib

ROOT=Path(__file__).resolve().parents[1]
CONFIG=ROOT/'config'
OUTPUT=ROOT/'docs'/'LIBRARY_PROGRESS.md'

def load():
    with (CONFIG/'reccmp-functions.csv').open(newline='',encoding='utf-8') as f:
        library=[r for r in csv.DictReader(f) if r['type']=='library']
    sizes={}
    with (CONFIG/'mapping.csv').open(newline='',encoding='utf-8') as f:
        for line,r in enumerate(csv.reader(f),1):
            if len(r)<3: raise ValueError(f'mapping.csv:{line}: expected at least three columns')
            sizes[int(r[1],0)]=int(r[2],0)
    manifest=tomllib.loads((CONFIG/'library-match-units.toml').read_text(encoding='utf-8'))
    units={u['name']:u for u in manifest.get('units',[])}
    with (CONFIG/'library-matches.csv').open(newline='',encoding='utf-8') as f:
        matches=list(csv.DictReader(f))
    return library,sizes,units,matches

def render()->str:
    library,sizes,units,matches=load()
    by_addr={int(r['address'],0):r for r in library}
    missing=[r for r in library if int(r['address'],0) not in sizes]
    if missing: raise ValueError(f'{len(missing)} library rows lack mapping sizes')
    accepted=[]
    seen=set()
    for row in matches:
        addr=int(row['address'],0)
        if addr in seen: raise ValueError(f'duplicate accepted library address {addr:#x}')
        seen.add(addr)
        inv=by_addr.get(addr)
        if inv is None: raise ValueError(f'accepted library row absent from inventory: {addr:#x}')
        unit=units.get(row['unit'])
        if unit is None: raise ValueError(f'accepted library row has unknown unit: {row["unit"]}')
        if row['status']!='matching' or not row['evidence']: raise ValueError(f'invalid acceptance at {addr:#x}')
        if row['name']!=inv['name'] or int(row['size'],0)!=sizes[addr] or int(unit['body_size'])!=sizes[addr]:
            raise ValueError(f'accepted name/size differs from inventory at {addr:#x}')
        accepted.append(row)
    total_bytes=sum(sizes[int(r['address'],0)] for r in library)
    exact_bytes=sum(int(r['size'],0) for r in accepted)
    configured_addrs={int(u['target_address']) for u in units.values()}
    configured_bytes=sum(sizes[a] for a in configured_addrs)
    fp=100*len(accepted)/len(library) if library else 0
    bp=100*exact_bytes/total_bytes if total_bytes else 0
    lines=[
        '# Library/runtime reconstruction progress','',
        'Generated independently from the authored progress ledger. Library exactness',
        'counts only rows accepted in `config/library-matches.csv` after archive-hash,',
        'COFF relocation, extent, and canonical-target replay.','',
        f'- Inventory-mapped library functions: **{len(library):,} / {len(library):,}**',
        f'- Inventory-mapped library bytes: **{total_bytes:,} / {total_bytes:,}**',
        f'- Configured library match units: **{len(units):,} functions / {configured_bytes:,} body bytes**',
        f'- Exact library functions: **{len(accepted):,} / {len(library):,} ({fp:.2f}%)**',
        f'- Exact library body bytes: **{exact_bytes:,} / {total_bytes:,} ({bp:.2f}%)**','',
        'Archive-member padding and compiler-owned associated bytes may be included in',
        '`compare_size`, but they do not inflate library body-byte progress.',''
    ]
    return '\n'.join(lines)

def main()->int:
    text=render()
    if '--check' in sys.argv:
        if not OUTPUT.exists() or OUTPUT.read_text(encoding='utf-8')!=text:
            print(f'stale generated library progress: {OUTPUT.relative_to(ROOT)}')
            return 1
        print('library progress artifact is current')
        return 0
    OUTPUT.write_text(text,encoding='utf-8')
    print(f'updated {OUTPUT.relative_to(ROOT)}')
    return 0
if __name__=='__main__': raise SystemExit(main())
