"""Shared validation for target-attested VC7 floating-point relocations."""

from __future__ import annotations

import re


REAL_LITERAL_SYMBOL = re.compile(r"^__real@([0-9A-Fa-f]{8}|[0-9A-Fa-f]{16})$")


def real_literal_bytes(relocation: dict[str, object]) -> bytes | None:
    """Decode and validate a VC7 ``__real`` relocation's literal bytes."""

    symbol = str(relocation.get("symbol", ""))
    match = REAL_LITERAL_SYMBOL.fullmatch(symbol)
    if match is None:
        if "data_hex" in relocation:
            raise ValueError("data_hex requires a 32-bit or 64-bit __real@... symbol")
        return None
    if str(relocation.get("type")) != "DIR32":
        raise ValueError("real literals must use DIR32 relocations")
    if int(relocation.get("addend", 0)) != 0:
        raise ValueError("real literals must use a zero addend")

    symbol_bytes = bytes.fromhex(match.group(1))[::-1]
    if "data_hex" not in relocation:
        return symbol_bytes

    data_hex = str(relocation["data_hex"])
    try:
        declared = bytes.fromhex(data_hex)
    except ValueError as exc:
        raise ValueError("data_hex must contain hexadecimal bytes") from exc
    if data_hex != declared.hex():
        raise ValueError("data_hex must use canonical lowercase hexadecimal")

    if declared != symbol_bytes:
        raise ValueError(
            f"{symbol} encodes {symbol_bytes.hex()}, not declared {data_hex}"
        )
    return symbol_bytes


def attested_real_literal_bytes(relocation: dict[str, object]) -> bytes | None:
    """Return explicitly recorded target bytes, or ``None`` when not recorded."""

    if "data_hex" not in relocation:
        return None
    return real_literal_bytes(relocation)
