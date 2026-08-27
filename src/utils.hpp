#pragma once

#include "inttypes.hpp"

#define ZUN_BIT(a) (1 << (a))

#define MAKE_FOURCC(a, b, c, d) ((a & 0xff) | ((b & 0xff) << 8) | ((c & 0xff) << 16) | ((d & 0xff) << 24))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define ARRAY_SIZE_SIGNED(x) ((i32)sizeof(x) / (i32)sizeof(x[0]))

namespace th08
{
namespace utils
{
void DebugPrint(char *fmt, ...);
void GuiDebugPrint(char *fmt, ...);
}; // namespace utils
}; // namespace th08
