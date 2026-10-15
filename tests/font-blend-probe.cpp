#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TH08_FONT_PROBE_WEIGHT
#define TH08_FONT_PROBE_WEIGHT FW_SEMIBOLD
#endif

static void require(bool result, const char *step)
{
    if (!result) { fprintf(stderr, "Failed %s\n", step); exit(1); }
}

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "Expected an output directory\n"); return 1; }
    const char *face = "\x82\x6C\x82\x72\x20\x83\x53\x83\x56\x83\x62\x83\x4E";
    const char *text = "\x8b\xbd";
    const COLORREF backgrounds[] = {RGB(0,0,0), RGB(255,255,255), RGB(17,91,203), RGB(127,61,199)};
    const COLORREF foregrounds[] = {RGB(255,255,255), RGB(0,0,0), RGB(31,140,241), RGB(241,18,61)};
    HDC dc = CreateCompatibleDC(NULL);
    require(dc != NULL, "device context");
    HFONT font = CreateFontA(28, 0, 0, 0, TH08_FONT_PROBE_WEIGHT, FALSE, FALSE, FALSE,
        SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
        FF_ROMAN | FIXED_PITCH, face);
    require(font != NULL, "text font");
    HGDIOBJ oldFont = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    int ascent = -1;
#ifndef TH08_MODERN_LINUX
    TEXTMETRICA metrics;
    require(GetTextMetricsA(dc, &metrics) != 0, "text metrics");
    ascent = metrics.tmAscent;
#endif
    bool first = true;
    unsigned id = 0;
    printf("[\n");
    for (int bits = 16; bits <= 32; bits += 16)
    {
        BITMAPINFO info;
        memset(&info, 0, sizeof(info));
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = 64;
        info.bmiHeader.biHeight = -64;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = bits;
        void *pixels = NULL;
        HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, NULL, 0);
        require(bitmap != NULL && pixels != NULL, "text bitmap");
        HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
        const unsigned size = 64 * 64 * bits / 8;
        for (unsigned bg = 0; bg < 4; ++bg)
        for (unsigned fg = 0; fg < 4; ++fg)
        for (unsigned mode = 0; mode < 3; ++mode, ++id)
        {
            unsigned char *bytes = static_cast<unsigned char *>(pixels);
            const unsigned red = backgrounds[bg] & 255, green = (backgrounds[bg] >> 8) & 255, blue = (backgrounds[bg] >> 16) & 255;
            for (unsigned pixel = 0; pixel < 64 * 64; ++pixel)
            {
                if (bits == 32)
                {
                    bytes[pixel * 4] = blue; bytes[pixel * 4 + 1] = green;
                    bytes[pixel * 4 + 2] = red; bytes[pixel * 4 + 3] = 0x5a;
                }
                else
                {
                    const unsigned packed = ((red >> 3) << 10) | ((green >> 3) << 5) | (blue >> 3) | 0x8000;
                    bytes[pixel * 2] = packed & 255; bytes[pixel * 2 + 1] = packed >> 8;
                }
            }
            const int x = mode == 1 ? -6 : 4, y = mode == 1 ? -8 : 4;
            SetTextColor(dc, foregrounds[fg]);
            require(TextOutA(dc, x, y, text, 2) != 0, "first text output");
            if (mode == 2)
            {
                SetTextColor(dc, foregrounds[fg] ^ 0xffffff);
                require(TextOutA(dc, 8, 8, text, 2) != 0, "overlapping text output");
            }
#ifndef TH08_MODERN_LINUX
            GdiFlush();
#endif
            char filename[2048];
            snprintf(filename, sizeof(filename), "%s/blend-%u.bin", argv[1], id);
            FILE *file = fopen(filename, "wb");
            require(file != NULL && fwrite(pixels, 1, size, file) == size, "bitmap output");
            require(fclose(file) == 0, "bitmap close");
            printf("%s{\"id\":%u,\"bits\":%d,\"background\":%lu,\"foreground\":%lu,\"mode\":%u,\"ascent\":%d}",
                first ? "" : ",\n", id, bits, static_cast<unsigned long>(backgrounds[bg]),
                static_cast<unsigned long>(foregrounds[fg]), mode, ascent);
            first = false;
        }
        SelectObject(dc, oldBitmap);
        DeleteObject(bitmap);
    }
    printf("\n]\n");
    SelectObject(dc, oldFont);
    DeleteObject(font);
    DeleteDC(dc);
    return 0;
}
