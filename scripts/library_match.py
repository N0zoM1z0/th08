"""Shared library archive/COFF comparison helpers for target-linked TH08 code."""
from __future__ import annotations
import hashlib, struct
from pathlib import Path
from coff import ObjectModule

AR_MAGIC=b"!<arch>\n"
RELOC_TYPES={"DIR32":0x0006,"REL32":0x0014}

def archive_members(data: bytes) -> dict[str, list[bytes]]:
    if not data.startswith(AR_MAGIC): raise ValueError("not an ar/COFF archive")
    pos=len(AR_MAGIC); longnames=b""; raw=[]
    while pos < len(data):
        if pos+60>len(data): raise ValueError("truncated archive header")
        h=data[pos:pos+60]; pos+=60
        if h[58:60]!=b"`\n": raise ValueError("invalid archive member trailer")
        name=h[:16].decode("ascii").rstrip(); size=int(h[48:58].decode("ascii").strip())
        body=data[pos:pos+size]
        if len(body)!=size: raise ValueError("truncated archive member")
        pos += size + (size & 1)
        if name == "//": longnames=body; continue
        raw.append((name,body))
    out={}
    for name,body in raw:
        if name in {"/", "/SYM64/"}: continue
        if name.startswith("/") and name[1:].isdigit():
            off=int(name[1:]); end=longnames.find(b"\x00",off)
            if end<0: raise ValueError("invalid long-name archive offset")
            name=longnames[off:end].decode("utf-8")
        else: name=name.rstrip("/")
        name=name.replace("\\","/")
        out.setdefault(name, []).append(body)
    return out

def verify_archive(path: Path, sha256: str) -> bytes:
    data=path.read_bytes(); actual=hashlib.sha256(data).hexdigest()
    if actual!=sha256: raise ValueError(f"archive hash mismatch: {actual}")
    return data

def coff_section_for_symbol(data: bytes, symbol_name: str):
    m=ObjectModule(); m.unpack(data)
    matches=[s for s in m.symbols if s.section_number>0 and s.get_name(m.string_table).decode("ascii")==symbol_name]
    if len(matches)!=1: raise ValueError(f"expected one section-defined symbol {symbol_name!r}, found {len(matches)}")
    sym=matches[0]; sec=m.sections[sym.section_number-1]
    if sec.data is None: raise ValueError("symbol section has no data")
    return m,sym,sec

def pe_bytes_at(data: bytes,address:int,size:int)->bytes:
    pe=struct.unpack_from("<I",data,0x3c)[0]
    if data[:2]!=b"MZ" or data[pe:pe+4]!=b"PE\0\0": raise ValueError("invalid PE target")
    count,opt_size=struct.unpack_from("<H12xH",data,pe+6); opt=pe+24; base=struct.unpack_from("<I",data,opt+28)[0]; table=opt+opt_size; rva=address-base
    for i in range(count):
        off=table+i*40; _,vs,sr,rs,ro=struct.unpack_from("<8sIIII",data,off)
        if sr<=rva and rva+size<=sr+max(vs,rs):
            rel=rva-sr
            if rel+size>rs: raise ValueError("target range exceeds PE raw section")
            return data[ro+rel:ro+rel+size]
    raise ValueError("target range outside PE sections")

def compare_member(member: bytes, unit: dict, target_code: bytes) -> dict:
    m,sym,sec=coff_section_for_symbol(member,str(unit["symbol"])); compare_size=int(unit["compare_size"])
    section_offset=int(unit.get("section_offset",0))
    section_size=int(unit.get("section_size",compare_size))
    aux=sym.aux_records[0] if sym.aux_records and hasattr(sym.aux_records[0],"total_size") else None
    allow_auxless=bool(unit.get("allow_auxless_comdat",False))
    if sym.value!=section_offset:
        raise ValueError(f"COFF symbol offset {sym.value:#x} != configured section_offset {section_offset:#x}")
    if len(sec.data)!=section_size:
        raise ValueError(f"archive section size {len(sec.data):#x} != configured section_size {section_size:#x}")
    if aux is None:
        if not allow_auxless:
            raise ValueError("COFF function lacks a function-definition aux record")
        IMAGE_SCN_CNT_CODE=0x00000020; IMAGE_SCN_LNK_COMDAT=0x00001000
        if section_offset!=0 or section_size!=compare_size:
            raise ValueError("auxless COMDAT comparison requires a whole-section extent")
        if sym.value!=0 or sym.type!=0x20 or not (sec.flags & IMAGE_SCN_CNT_CODE) or not (sec.flags & IMAGE_SCN_LNK_COMDAT):
            raise ValueError("auxless symbol is not an offset-zero function in a code COMDAT")
        owners=[]
        for candidate in m.symbols:
            if candidate.section_number==sym.section_number and candidate.value==0 and candidate.type==0x20:
                owners.append(candidate.get_name(m.string_table).decode("ascii",errors="strict"))
        if owners != [str(unit["symbol"])]:
            raise ValueError(f"auxless COMDAT section has ambiguous function owners: {owners!r}")
    elif allow_auxless:
        raise ValueError("allow_auxless_comdat is set for a symbol with a function-definition aux record")
    if section_offset or section_size!=compare_size:
        if int(aux.total_size)!=compare_size:
            raise ValueError(f"COFF aux extent {int(aux.total_size):#x} != compare_size {compare_size:#x}")
    if section_offset+compare_size>section_size:
        raise ValueError("configured library subrange extends beyond archive section")
    code=bytearray(sec.data[section_offset:section_offset+compare_size])
    actual=[]
    for r in sec.relocations:
        local_offset=r.virtual_address-section_offset
        if 0<=local_offset<compare_size:
            ref=m.symbols[r.symbol_table_index].get_name(m.string_table).decode("ascii")
            actual.append((local_offset,r.type,ref))
    expected=[]
    for r in unit.get("relocations",[]): expected.append((int(r["offset"]),RELOC_TYPES[str(r["type"])],str(r["symbol"])))
    if sorted(actual)!=sorted(expected): raise ValueError(f"COFF relocations differ: actual={sorted(actual)!r} expected={sorted(expected)!r}")
    replay=[]
    for r in unit.get("relocations",[]):
        off=int(r["offset"]); kind=str(r["type"]); add=struct.unpack_from("<I",code,off)[0]; base=int(r["target_base"])
        if kind=="DIR32": value=base+add
        elif kind=="REL32": value=base+add-(int(unit["target_address"])+off+4)
        else: raise ValueError(f"unsupported relocation {kind}")
        struct.pack_into("<I",code,off,value & 0xffffffff)
        replay.append({"offset":f"0x{off:X}","type":kind,"symbol":r["symbol"],"target_base":f"0x{base:08X}","addend":f"0x{add:08X}"})
    diffs=[i for i,(a,b) in enumerate(zip(code,target_code)) if a!=b]
    return {"result":"exact" if not diffs else "mismatch","compare_size":compare_size,"body_size":int(unit["body_size"]),"differences":len(diffs),"first_difference":None if not diffs else f"0x{diffs[0]:X}","relocations":replay}
