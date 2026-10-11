#!/usr/bin/env python3
"""Reject relocation bindings that replace keyboard input with replay input."""
import unittest
from match_inputs import INPUT_GLOBALS, validate_input_global


class InputBindingTests(unittest.TestCase):
    def test_canonical_words(self):
        for symbol, target in INPUT_GLOBALS.items():
            validate_input_global(dict(symbol=symbol, type="DIR32", target=target))

    def test_swapped_input_words(self):
        for symbol, target in INPUT_GLOBALS.items():
            other = next(value for value in INPUT_GLOBALS.values() if value != target)
            with self.assertRaisesRegex(ValueError, "must resolve to its input word"):
                validate_input_global(dict(symbol=symbol, type="DIR32", target=other))

    def test_input_word_cannot_be_a_call(self):
        for symbol, target in INPUT_GLOBALS.items():
            with self.assertRaises(ValueError):
                validate_input_global(dict(symbol=symbol, type="REL32", target=target))


if __name__ == "__main__":
    unittest.main()
