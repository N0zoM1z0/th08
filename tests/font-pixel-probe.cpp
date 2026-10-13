#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(bool result, const char *step)
{
    if (!result) { fprintf(stderr, "Failed %s\n", step); exit(1); }
}

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "Expected an image output directory\n"); return 1; }
    const char *face = "\x82\x6C\x82\x72\x20\x83\x53\x83\x56\x83\x62\x83\x4E";
    const int heights[] = {14, 18, 22, 26, 28, 30, 32, -14, -28};
    const char *texts[] = {
        "Stage 3",
        "\x8d\x91\x91\xcc\x81\x75\x8e\x4f\x8e\xed\x82\xcc\x90\x5f\x8a\xed\x81\x40\x8b\xbd\x81\x76",
        "\x96\xa2\x97\x88\x81\x75\x8d\x82\x93\x56\x8c\xb4\x81\x76"
    };
    const int width = 768, height = 96, pitch = width * 4;
    BITMAPINFO info;
    memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    HDC dc = CreateCompatibleDC(NULL);
    require(dc != NULL, "device context");
    void *pixels = NULL;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, NULL, 0);
    require(bitmap != NULL && pixels != NULL, "text bitmap");
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    printf("[\n");
    bool first = true;
    for (unsigned h = 0; h < sizeof(heights) / sizeof(heights[0]); ++h)
    {
        HFONT font = CreateFontA(heights[h], 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
            FF_ROMAN | FIXED_PITCH, face);
        require(font != NULL, "text font");
        HGDIOBJ oldFont = SelectObject(dc, font);
#ifndef TH08_MODERN_LINUX
        WCHAR faceName[256]; char faceUtf8[1024];
        require(GetTextFaceW(dc, 256, faceName) != 0, "selected font face");
        require(WideCharToMultiByte(CP_UTF8, 0, faceName, -1, faceUtf8, sizeof(faceUtf8), NULL, NULL) != 0,
                "font face encoding");
#endif
        for (unsigned t = 0; t < sizeof(texts) / sizeof(texts[0]); ++t)
        {
            memset(pixels, 0, pitch * height);
            require(TextOutA(dc, 4, 4, texts[t], strlen(texts[t])) != 0, "text output");
#ifndef TH08_MODERN_LINUX
            GdiFlush();
#endif
            char filename[2048];
            snprintf(filename, sizeof(filename), "%s/text-%d-%u.ppm", argv[1], heights[h], t);
            FILE *image = fopen(filename, "wb");
            require(image != NULL, "text image file");
            fprintf(image, "P6\n%d %d\n255\n", width, height);
            int left = width, top = height, right = -1, bottom = -1, count = 0;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const BYTE *pixel = static_cast<const BYTE *>(pixels) + y * pitch + x * 4;
                    const BYTE rgb[] = {pixel[2], pixel[1], pixel[0]};
                    require(fwrite(rgb, 1, 3, image) == 3, "text image pixel");
                    if (rgb[0] || rgb[1] || rgb[2])
                    {
                        if (x < left) left = x;
                        if (x > right) right = x;
                        if (y < top) top = y;
                        if (y > bottom) bottom = y;
                        ++count;
                    }
                }
            }
            require(fclose(image) == 0 && count > 0, "nonempty text image");
            printf("%s{\"requestedHeight\":%d,\"text\":%u,\"bounds\":[%d,%d,%d,%d],\"pixels\":%d",
                first ? "" : ",\n", heights[h], t, left, top, right, bottom, count);
#ifndef TH08_MODERN_LINUX
            printf(",\"face\":\"%s\"", faceUtf8);
#endif
            printf("}");
            first = false;
        }
        SelectObject(dc, oldFont);
        DeleteObject(font);
    }
    printf("\n]\n");
    SelectObject(dc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
    return 0;
}
