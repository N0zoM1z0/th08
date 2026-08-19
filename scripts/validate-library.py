#!/usr/bin/env python3
"""Validate target-linked library provenance, match units, and accepted ledger.

Examples:
  python3 scripts/validate-library.py
  python3 scripts/validate-library.py --require-archives
"""
from __future__ import annotations
import argparse,csv,hashlib,re,tomllib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]; CONFIG=ROOT/'config'

def main()->int:
 p=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter); p.add_argument('--require-archives',action='store_true'); a=p.parse_args(); errors=[]
 prov=tomllib.loads((CONFIG/'library-provenance.toml').read_text()); units=tomllib.loads((CONFIG/'library-match-units.toml').read_text())
 if prov.get('schema_version')!=1: errors.append('unsupported library provenance schema')
 if units.get('schema_version')!=1: errors.append('unsupported library match-unit schema')
 archives={}
 for x in prov.get('archives',[]):
  i=str(x.get('id','')); sha=str(x.get('sha256','')); path=ROOT/str(x.get('path',''))
  if not i or i in archives: errors.append(f'duplicate/empty archive id {i!r}')
  if not re.fullmatch(r'[0-9a-f]{64}',sha): errors.append(f'{i}: invalid sha256')
  archives[i]=x
  if a.require_archives:
   if not path.is_file(): errors.append(f'{i}: archive missing: {path}')
   elif hashlib.sha256(path.read_bytes()).hexdigest()!=sha: errors.append(f'{i}: archive hash mismatch')
 mapping={}
 with (CONFIG/'mapping.csv').open(newline='') as f:
  for r in csv.reader(f): mapping[int(r[1],0)]=(r[0],int(r[2],0))
 lib={}
 with (CONFIG/'reccmp-functions.csv').open(newline='') as f:
  for r in csv.DictReader(f):
   if r['type']=='library': lib[int(r['address'],0)]=r['name']
 names={}; by_addr={}
 for u in units.get('units',[]):
  name=str(u.get('name','')); addr=int(u.get('target_address',0)); body=int(u.get('body_size',0)); comp=int(u.get('compare_size',0))
  if not name or name in names: errors.append(f'duplicate/empty unit {name!r}')
  names[name]=u; by_addr[addr]=name
  if u.get('archive') not in archives: errors.append(f'{name}: unknown archive')
  if addr not in lib: errors.append(f'{name}: target address is not library inventory')
  if addr not in mapping or mapping[addr][1]!=body: errors.append(f'{name}: body_size differs from mapping')
  if comp<body or not u.get('member') or not u.get('symbol'): errors.append(f'{name}: invalid comparison extent/member/symbol')
  section_offset=int(u.get('section_offset',0)); section_size=int(u.get('section_size',comp)); allow_auxless=u.get('allow_auxless_comdat',False); allow_tail=u.get('allow_tail_local_funclet',False)
  if section_offset<0 or section_size<section_offset+comp: errors.append(f'{name}: invalid section_offset/section_size')
  if ('section_offset' in u) != ('section_size' in u): errors.append(f'{name}: shared-section units must pin both section_offset and section_size')
  if not isinstance(allow_auxless,bool): errors.append(f'{name}: allow_auxless_comdat must be boolean')
  if not isinstance(allow_tail,bool): errors.append(f'{name}: allow_tail_local_funclet must be boolean')
  if allow_auxless and allow_tail: errors.append(f'{name}: cannot combine auxless and tail-local-funclet modes')
  if allow_auxless and ('section_offset' in u or 'section_size' in u): errors.append(f'{name}: auxless COMDAT units must use whole-section comparison')
  if allow_tail:
   if 'section_offset' not in u or 'section_size' not in u or not u.get('owner_symbol'): errors.append(f'{name}: tail local funclet must pin owner_symbol and section bounds')
   if comp!=body or section_offset<=0 or section_offset+comp!=section_size: errors.append(f'{name}: tail local funclet must exactly consume a nonzero section tail')
  seen=set()
  for r in u.get('relocations',[]):
   key=(int(r['offset']),str(r['type']),str(r['symbol']))
   if key in seen: errors.append(f'{name}: duplicate relocation {key!r}')
   seen.add(key)
   if str(r['type']) not in {'DIR32','REL32'} or not (0<=int(r['offset'])<=comp-4): errors.append(f'{name}: invalid relocation {key!r}')
 with (CONFIG/'library-matches.csv').open(newline='') as f:
  rd=csv.DictReader(f); expected=['address','name','size','status','unit','evidence']
  if rd.fieldnames!=expected: errors.append(f'library-matches.csv: unexpected columns {rd.fieldnames}')
  accepted=0
  seen_units=set()
  for row in rd:
   addr=int(row['address'],0); unit=row['unit']
   if row['address'] != f'0x{addr:08X}': errors.append(f'{unit}: noncanonical accepted address {row["address"]!r}')
   if unit in seen_units: errors.append(f'duplicate accepted unit {unit}')
   seen_units.add(unit)
   if unit not in names: errors.append(f'accepted row unknown unit {unit}') ; continue
   u=names[unit]
   if addr!=int(u['target_address']) or int(row['size'],0)!=int(u['body_size']): errors.append(f'{unit}: accepted address/size mismatch')
   if row['name']!=lib.get(addr) or row['status']!='matching' or not row['evidence']: errors.append(f'{unit}: invalid accepted row')
   accepted+=1
 if errors:
  for e in errors: print('error:',e)
  return 1
 print(f'library tracking OK: {len(archives)} archives, {len(names)} match units, {accepted} accepted matches')
 return 0
if __name__=='__main__': raise SystemExit(main())
