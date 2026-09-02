#!/usr/bin/env python3
"""Contract tests for VC7 floating-point relocation validation."""

from __future__ import annotations

from match_literals import attested_real_literal_bytes, real_literal_bytes


def expect_error(relocation: dict[str, object], message: str) -> None:
    try:
        real_literal_bytes(relocation)
    except ValueError as exc:
        assert message in str(exc), str(exc)
    else:
        raise AssertionError(f"expected rejection for {relocation!r}")


def main() -> int:
    inferred = {"type": "DIR32", "symbol": "__real@3f800000"}
    assert real_literal_bytes(inferred) == bytes.fromhex("0000803f")
    assert attested_real_literal_bytes(inferred) is None

    recorded = {**inferred, "data_hex": "0000803f"}
    assert real_literal_bytes(recorded) == bytes.fromhex("0000803f")
    assert attested_real_literal_bytes(recorded) == bytes.fromhex("0000803f")

    inferred_double = {
        "type": "DIR32",
        "symbol": "__real@4060000000000000",
    }
    assert real_literal_bytes(inferred_double) == bytes.fromhex(
        "0000000000006040"
    )

    assert real_literal_bytes({"type": "REL32", "symbol": "ordinary"}) is None
    expect_error({**inferred, "type": "REL32"}, "must use DIR32")
    expect_error({**inferred, "addend": 4}, "zero addend")
    expect_error({**inferred, "data_hex": "00000000"}, "not declared")
    expect_error(
        {"type": "DIR32", "symbol": "ordinary", "data_hex": "0000803f"},
        "data_hex requires",
    )

    print("Relocation literal validation contract tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
