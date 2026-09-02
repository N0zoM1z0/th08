"""Shared validation for target-attested VC7 floating-point relocations."""

from __future__ import annotations

import re


REAL_LITERAL_SYMBOL = re.compile(r"^__real@([0-9A-Fa-f]{8}|[0-9A-Fa-f]{16})$")


def attested_real_literal_bytes(relocation: dict[str, object]) -> bytes | None:
    """Return declared target bytes, or ``None`` for an unattested relocation."""

    if "data_hex" not in relocation:
        return None
    if str(relocation.get("type")) != "DIR32":
        raise ValueError("data_hex is supported only for DIR32 real literals")
    if int(relocation.get("addend", 0)) != 0:
        raise ValueError("attested real literals must use a zero addend")

    symbol = str(relocation.get("symbol", ""))
    match = REAL_LITERAL_SYMBOL.fullmatch(symbol)
    if match is None:
        raise ValueError("data_hex requires a 32-bit or 64-bit __real@... symbol")

    data_hex = str(relocation["data_hex"])
    try:
        declared = bytes.fromhex(data_hex)
    except ValueError as exc:
        raise ValueError("data_hex must contain hexadecimal bytes") from exc
    if data_hex != declared.hex():
        raise ValueError("data_hex must use canonical lowercase hexadecimal")

    symbol_bytes = bytes.fromhex(match.group(1))[::-1]
    if declared != symbol_bytes:
        raise ValueError(
            f"{symbol} encodes {symbol_bytes.hex()}, not declared {data_hex}"
        )
    return declared
