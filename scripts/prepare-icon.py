#!/usr/bin/env python3
"""Prepare the deterministic single-image placeholder used by the TH08 resource.

The tracked placeholder contains several indexed icon sizes for convenient
display in modern tools.  TH08 1.00d's resource tree instead contains one
32x32, 24-bit icon image.  This build-internal generator preserves the tracked
placeholder artwork while emitting that target-observed container shape; it
does not read or copy the original executable's icon payload.

This file deliberately uses syntax accepted by the older Python 3 interpreter
inside the VC7/Wine build prefix.

Example:
  python3 scripts/prepare-icon.py resources/placeholder.ico build/icon.ico
"""

import argparse
import os
import struct
import sys


class IconFormatError(ValueError):
    pass


def indexed_pixel(row, x, bit_count):
    if bit_count == 8:
        return row[x]
    if bit_count == 4:
        value = row[x // 2]
        return value >> 4 if x % 2 == 0 else value & 0x0F
    if bit_count == 1:
        return (row[x // 8] >> (7 - x % 8)) & 1
    raise IconFormatError("unsupported indexed bit depth: {}".format(bit_count))


def prepare_icon(source, width):
    if len(source) < 6:
        raise IconFormatError("truncated ICO header")
    reserved, icon_type, count = struct.unpack_from("<HHH", source, 0)
    if reserved != 0 or icon_type != 1 or count == 0:
        raise IconFormatError("input is not a non-empty icon file")
    if len(source) < 6 + count * 16:
        raise IconFormatError("truncated ICO directory")

    selected = None
    for index in range(count):
        offset = 6 + index * 16
        width_byte, height_byte, _colors, _reserved, _planes, _bits, size, data_offset = (
            struct.unpack_from("<BBBBHHII", source, offset)
        )
        entry_width = width_byte or 256
        entry_height = height_byte or 256
        if entry_width == width and entry_height == width:
            selected = (data_offset, size, entry_width)
            break
    if selected is None:
        raise IconFormatError("input has no {0}x{0} icon image".format(width))

    data_offset, data_size, selected_width = selected
    if data_offset + data_size > len(source):
        raise IconFormatError("selected icon image is truncated")
    dib = source[data_offset : data_offset + data_size]
    if len(dib) < 40:
        raise IconFormatError("selected image lacks a BITMAPINFOHEADER")
    (
        header_size,
        dib_width,
        doubled_height,
        planes,
        bit_count,
        compression,
        _image_size,
        x_pixels_per_meter,
        y_pixels_per_meter,
        colors_used,
        _colors_important,
    ) = struct.unpack_from("<IiiHHIIiiII", dib, 0)
    if header_size < 40 or header_size > len(dib):
        raise IconFormatError("unsupported DIB header size")
    if dib_width != selected_width or doubled_height != selected_width * 2:
        raise IconFormatError("selected image has inconsistent DIB dimensions")
    if planes != 1 or bit_count not in (1, 4, 8) or compression != 0:
        raise IconFormatError("selected image must be an uncompressed indexed DIB")

    palette_count = colors_used or (1 << bit_count)
    palette_offset = header_size
    palette_size = palette_count * 4
    if palette_offset + palette_size > len(dib):
        raise IconFormatError("selected image has a truncated palette")
    palette = [
        struct.unpack_from("<BBBB", dib, palette_offset + index * 4)[:3]
        for index in range(palette_count)
    ]

    xor_row_size = ((selected_width * bit_count + 31) // 32) * 4
    mask_row_size = ((selected_width + 31) // 32) * 4
    xor_size = xor_row_size * selected_width
    mask_size = mask_row_size * selected_width
    pixels_offset = palette_offset + palette_size
    if pixels_offset + xor_size + mask_size > len(dib):
        raise IconFormatError("selected image has truncated XOR or AND pixels")

    output_row_size = ((selected_width * 24 + 31) // 32) * 4
    output_pixels = bytearray()
    for row_index in range(selected_width):
        row = dib[
            pixels_offset + row_index * xor_row_size :
            pixels_offset + (row_index + 1) * xor_row_size
        ]
        output_row = bytearray()
        for x in range(selected_width):
            palette_index = indexed_pixel(row, x, bit_count)
            if palette_index >= len(palette):
                raise IconFormatError("pixel references an absent palette entry")
            blue, green, red = palette[palette_index]
            output_row.extend((blue, green, red))
        output_row.extend(b"\0" * (output_row_size - len(output_row)))
        output_pixels.extend(output_row)

    mask_offset = pixels_offset + xor_size
    mask = dib[mask_offset : mask_offset + mask_size]
    output_dib = struct.pack(
        "<IiiHHIIiiII",
        40,
        selected_width,
        selected_width * 2,
        1,
        24,
        0,
        # BI_RGB permits zero here, and the canonical 32x32 DIB uses zero.
        0,
        x_pixels_per_meter,
        y_pixels_per_meter,
        0,
        0,
    ) + bytes(output_pixels) + mask

    icon_header_size = 6 + 16
    icon_header = struct.pack("<HHH", 0, 1, 1)
    icon_entry = struct.pack(
        "<BBBBHHII",
        selected_width,
        selected_width,
        0,
        0,
        1,
        24,
        len(output_dib),
        icon_header_size,
    )
    return icon_header + icon_entry + output_dib


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source")
    parser.add_argument("output")
    parser.add_argument("--width", type=int, default=32)
    args = parser.parse_args()
    try:
        with open(args.source, "rb") as handle:
            output = prepare_icon(handle.read(), args.width)
        output_directory = os.path.dirname(args.output)
        if output_directory and not os.path.isdir(output_directory):
            os.makedirs(output_directory)
        with open(args.output, "wb") as handle:
            handle.write(output)
    except (IOError, OSError, IconFormatError, struct.error) as exc:
        print("error: {}".format(exc), file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
