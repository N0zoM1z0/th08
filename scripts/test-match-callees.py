#!/usr/bin/env python3
"""Reject a comparison manifest that substitutes sine for cosine or vice versa."""
import unittest
from match_callees import MATH_CALLEES, validate_math_callee


class CalleeTests(unittest.TestCase):
    def test_retail_wrapper_bindings(self):
        for symbol, target in MATH_CALLEES.items():
            validate_math_callee(dict(symbol=symbol, type="REL32", target=target))

    def test_swapped_wrapper_bindings(self):
        for symbol, target in MATH_CALLEES.items():
            other = next(value for value in MATH_CALLEES.values() if value != target)
            with self.assertRaisesRegex(ValueError, "must resolve to its retail wrapper"):
                validate_math_callee(dict(symbol=symbol, type="REL32", target=other))


if __name__ == "__main__":
    unittest.main()
