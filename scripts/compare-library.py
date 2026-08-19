#!/usr/bin/env python3
"""Compare one SHA-pinned archive member against the canonical TH08 target.

Examples:
  python3 scripts/compare-library.py --list
  python3 scripts/compare-library.py d3dx-sse-vec3-normalize --json
"""
from __future__ import annotations
import argparse,hashlib,json,tomllib
from pathlib import Path
from library_match import archive_members,compare_member,pe_bytes_at,verify_archive
ROOT=Path(__file__).resolve().parents[1]
def main()->int:
 p=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter); p.add_argument('unit',nargs='?'); p.add_argument('--list',action='store_true'); p.add_argument('--json',action='store_true'); a=p.parse_args()
 prov=tomllib.loads((ROOT/'config/library-provenance.toml').read_text()); manifest=tomllib.loads((ROOT/'config/library-match-units.toml').read_text()); archives={x['id']:x for x in prov['archives']}; units={x['name']:x for x in manifest['units']}
 if a.list:
  print('\n'.join(units)); return 0
 if not a.unit or a.unit not in units: p.error('select a configured unit')
 u=units[a.unit]; ar=archives[u['archive']]; path=ROOT/ar['path']; data=verify_archive(path,ar['sha256']); members=archive_members(data); member_name=str(u['member'])
 if member_name not in members: raise SystemExit(f'archive member missing: {member_name}')
 candidates=[]
 for member in members[member_name]:
  try:
   from library_match import coff_section_for_symbol
   coff_section_for_symbol(member,str(u['symbol']))
  except ValueError:
   continue
  candidates.append(member)
 if len(candidates)!=1: raise SystemExit(f'archive member identity ambiguous for {member_name}: {len(candidates)} symbol-owning candidates')
 target_path=ROOT/'resources/th08.exe'; target=target_path.read_bytes(); expected=tomllib.loads((ROOT/'config/target.toml').read_text())['target']; digest=hashlib.sha256(target).hexdigest()
 if len(target)!=int(expected['size']) or digest!=expected['sha256']: raise SystemExit('target identity mismatch')
 tc=pe_bytes_at(target,int(u['target_address']),int(u['compare_size'])); report=compare_member(candidates[0],u,tc); report.update({'unit':a.unit,'archive':u['archive'],'member':member_name,'target_address':f"0x{int(u['target_address']):08X}"})
 print(json.dumps(report,indent=2) if a.json else f"{a.unit}: {report['result']} ({report['body_size']:#x} body / {report['compare_size']:#x} compared)")
 return 0 if report['result']=='exact' else 1
if __name__=='__main__': raise SystemExit(main())
