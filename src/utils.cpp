#include "th_pch.h"

#ifdef DEBUG
#include <cstdarg>
#include <stdio.h>
#endif

#include "utils.hpp"

namespace th08
{
namespace utils
{
void DebugPrint(char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    std::va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    va_end(args);

    printf("DEBUG: %s\n", tmpBuffer);
#endif
}

void GuiDebugPrint(char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    std::va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    va_end(args);

    printf("DEBUG: %s\n", tmpBuffer);
#endif
}
}; // namespace utils
}; // namespace th08
