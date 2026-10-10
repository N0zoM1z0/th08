"""Keep physical keyboard input and replay-fed gameplay input distinct."""

# RecordInputAndFps reads physical input and copies it to the gameplay word.
# PlaybackInputAndFps writes the gameplay word from the replay stream.
INPUT_GLOBALS = {
    "?g_CurFrameInput@th08@@3GA": 0x0164D528,
    "?g_GuiMessageInputCurrent@th08@@3GA": 0x0164D52C,
}


def validate_input_global(relocation):
    symbol = relocation.get("symbol")
    expected = INPUT_GLOBALS.get(symbol)
    if expected is not None and (relocation["type"] != "DIR32" or int(relocation["target"]) != expected):
        raise ValueError(f"{symbol} must resolve to its input word at 0x{expected:08X}")
