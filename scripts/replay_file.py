"""Read TH08 version-6 replay files without running or changing the game.

Layout and codec: ReplayManager::LoadReplayData (1.00d, 0x451d90),
ReplayManager::SaveReplay, and Lzss::Decode (0x4740e0).
"""
import hashlib
import struct
from pathlib import Path

STAGES = ("Stage 1", "Stage 2", "Stage 3", "Stage 4A", "Stage 4B", "Stage 5",
          "Final A", "Final B", "Extra")
SHOTS = ("Border Team", "Magic Team", "Scarlet Team", "Ghost Team", "Reimu", "Yukari",
         "Marisa", "Alice", "Sakuya", "Remilia", "Youmu", "Yuyuko")
DIFFICULTIES = ("Easy", "Normal", "Hard", "Lunatic", "Extra")
RETAIL_IDENTITY = (840704, 2724749753, "0100d")


def valid_input_tail(remaining, final_stage):
    # RecordInputAndFps (0x452310) keeps three stage-clear inputs.
    # StopRecording (0x4531a0), also called by SaveReplay, extends the final
    # stream. Complete retail captures of the pinned fixtures leave 2/3 and
    # 6/7 records respectively; an exact count is established by playback.
    return remaining in ((6, 7) if final_stage else (2, 3))


def decompress(data, expected_size):
    if not 0 <= expected_size <= 0x400000:
        raise ValueError("Replay decompressed size exceeds the game's 4 MiB buffer")
    bits = 0
    dictionary = bytearray(8192)
    head = 1
    result = bytearray()

    def take(count):
        nonlocal bits
        # Retail supplies zero bits after the compressed extent. Its encoder
        # can leave the zero-offset terminator partly in that implicit tail.
        if bits + count > len(data) * 8 + 14:
            raise ValueError("Truncated replay LZSS stream")
        value = 0
        for _ in range(count):
            value = (value << 1) | (((data[bits // 8] >> (7 - bits % 8)) & 1)
                                    if bits < len(data) * 8 else 0)
            bits += 1
        return value

    def emit(value):
        nonlocal head
        if len(result) >= expected_size:
            raise ValueError("Replay LZSS stream exceeds its declared size")
        result.append(value)
        dictionary[head] = value
        head = (head + 1) & 8191

    while True:
        if take(1):
            emit(take(8))
        else:
            offset = take(13)
            if not offset:
                break
            for index in range(take(4) + 3):
                emit(dictionary[(offset + index) & 8191])
    if len(result) != expected_size:
        raise ValueError("Replay LZSS output size differs from its header")
    return bytes(result)


def decode(data):
    if len(data) < 0x68 or data[:6] != b"T8RP\x06\x00":
        raise ValueError("Expected a TH08 version-6 replay")
    decoded = bytearray(data)
    file_size, checksum = struct.unpack_from("<II", decoded, 0x0c)
    if not 0x68 <= file_size <= len(data):
        raise ValueError("Invalid replay file extent")
    key = decoded[0x15]
    for index in range(0x18, file_size):
        decoded[index] = (decoded[index] - key) & 255
        key = (key + 7) & 255
    if (0x3f000318 + sum(decoded[0x15:file_size])) & 0xffffffff != checksum:
        raise ValueError("Replay checksum mismatch")
    compressed, expanded = struct.unpack_from("<II", decoded, 0x18)
    if compressed != file_size - 0x68:
        raise ValueError("Invalid replay compressed extent")
    payload = decompress(decoded[0x68:file_size], expanded)
    if len(payload) < 0x134 - 0x68:
        raise ValueError("Truncated replay metadata")
    return bytes(decoded[:0x68]) + payload


def inspect(path, require_retail=True):
    raw = Path(path).read_bytes()
    data = decode(raw)
    identity = (*struct.unpack_from("<II", data, 0x124),
                data[0x12c:0x132].split(b"\0", 1)[0].decode("ascii"))
    if require_retail and identity != RETAIL_IDENTITY:
        raise ValueError(f"Replay executable identity is {identity}, expected {RETAIL_IDENTITY}")
    shot, difficulty = data[0x6a:0x6c]
    if shot >= len(SHOTS) or difficulty >= len(DIFFICULTIES):
        raise ValueError("Invalid replay shot or difficulty")
    if data[0xb4 + 0x25]:
        raise ValueError("Slow-mode replay")
    stage_offsets = struct.unpack_from("<9I", data, 0x20)
    fps_offsets = struct.unpack_from("<9I", data, 0x44)
    offsets = sorted(value for value in (*stage_offsets, *fps_offsets) if value)
    if len(set(offsets)) != len(offsets) or any(not 0x134 <= value < len(data) for value in offsets):
        raise ValueError("Invalid replay stage/FPS offsets")
    extended = data[6]
    if extended not in (0, 1):
        raise ValueError("Invalid replay input encoding")
    stride = 6 if extended else 2
    stages = []
    for stage, offset in enumerate(stage_offsets):
        if not offset:
            if fps_offsets[stage]:
                raise ValueError("FPS stream without a replay stage")
            continue
        end = min(value for value in (*offsets, len(data)) if value > offset)
        size = end - offset - 0x24
        if size < stride or size % stride or not fps_offsets[stage]:
            raise ValueError("Invalid replay input extent")
        stages.append(dict(index=stage, name=STAGES[stage], offset=offset,
                           inputRecords=size // stride, endScore=struct.unpack_from("<I", data, offset)[0],
                           rngSeed=struct.unpack_from("<H", data, offset + 0x1a)[0]))
    if not stages:
        raise ValueError("Replay contains no stages")
    return dict(sha256=hashlib.sha256(raw).hexdigest(), bytes=len(raw),
                shot=shot, shotName=SHOTS[shot], difficulty=difficulty,
                difficultyName=DIFFICULTIES[difficulty], extended=bool(extended),
                practice=bool(data[0x7b]), spellcard=struct.unpack_from("<h", data, 0x7c)[0],
                clearState=data[0x11c], executable=dict(size=identity[0], checksum=identity[1], version=identity[2]),
                stages=stages, player=data[0x72:0x7a].rstrip(b"\0").decode("cp932", errors="replace"))


if __name__ == "__main__":
    import argparse
    import json
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("replay", type=Path)
    parser.add_argument("--allow-other-version", action="store_true")
    args = parser.parse_args()
    print(json.dumps(inspect(args.replay, not args.allow_other_version), indent=2, ensure_ascii=True))
