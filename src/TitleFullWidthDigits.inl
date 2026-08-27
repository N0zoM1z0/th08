// Shared production/probe owner for the full-width number formatter used by
// spell-card information.
#pragma var_order(i, multiplier)
char *__fastcall ConvertToFullWidthDigits(i32 value, i32 width)
{
    i32 multiplier;
    i32 i;
    multiplier = 1;
    memset(g_FullWidthNumberBuffer, 0, sizeof(g_FullWidthNumberBuffer));
    for (i = 1; i < width; i++)
        multiplier *= 10;
    i = 0;
    while (multiplier > 0)
    {
        strcpy(&g_FullWidthNumberBuffer[i], g_FullWidthDigits[value / multiplier]);
        value %= multiplier;
        multiplier /= 10;
        i += 2;
    }
    return g_FullWidthNumberBuffer;
}
