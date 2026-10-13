#include <windows.h>
#include <stdio.h>

int main() {
    const char *face = "\x82\x6C\x82\x72\x20\x83\x53\x83\x56\x83\x62\x83\x4E";
    const int heights[] = {14, 18, 22, 26, 30, 32};
    HDC dc = CreateCompatibleDC(NULL);
    if (!dc) return 1;
    printf("[\n");
    for (unsigned i = 0; i < sizeof(heights) / sizeof(heights[0]); ++i) {
        HFONT font = CreateFontA(heights[i], 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE,
            FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, FF_ROMAN | FIXED_PITCH, face);
        if (!font) return 2;
        HGDIOBJ old = SelectObject(dc, font);
        WCHAR name[256]; char utf8[1024]; TEXTMETRICA metrics; SIZE size;
        if (!GetTextFaceW(dc, 256, name) || !GetTextMetricsA(dc, &metrics) ||
            !GetTextExtentPoint32A(dc, "Stage 3", 7, &size)) return 3;
        if (!WideCharToMultiByte(CP_UTF8, 0, name, -1, utf8, sizeof(utf8), NULL, NULL)) return 4;
        printf("%s{\"requestedHeight\":%d,\"face\":\"%s\",\"height\":%ld,"
            "\"ascent\":%ld,\"descent\":%ld,\"internalLeading\":%ld,"
            "\"averageWidth\":%ld,\"weight\":%ld,\"textWidth\":%ld,\"textHeight\":%ld}",
            i ? ",\n" : "", heights[i], utf8, metrics.tmHeight, metrics.tmAscent,
            metrics.tmDescent, metrics.tmInternalLeading, metrics.tmAveCharWidth,
            metrics.tmWeight, size.cx, size.cy);
        SelectObject(dc, old); DeleteObject(font);
    }
    printf("\n]\n"); DeleteDC(dc); return 0;
}
