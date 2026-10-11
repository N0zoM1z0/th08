"""Check the identities of target-pinned VC7 inline math callees."""

# The retail wrappers call __CIcos / __CIsin respectively. See mapping.csv
# and RUNTIME_ISSUES.md RT-009 for the disassembly and replay evidence.
MATH_CALLEES = {"@cosf@4": 0x00408D40, "@sinf@4": 0x00409060}


def validate_math_callee(relocation):
    symbol = relocation.get("symbol")
    expected = MATH_CALLEES.get(symbol)
    if expected is not None and int(relocation["target"]) != expected:
        raise ValueError(
            f"{symbol} must resolve to its retail wrapper at 0x{expected:08X}, "
            f"not 0x{int(relocation['target']):08X}"
        )
