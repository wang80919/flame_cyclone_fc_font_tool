#pragma once
#include <windows.h>
#include <stdint.h>

#define WM_PALETTE_CHANGED                      (WM_USER + 1)
#define WM_LOAD_FILE                            (WM_USER + 2)
#define WM_TILE_CHANGED                         (WM_USER + 3)
#define WM_TILE_CLEAR                           (WM_USER + 4)
#define WM_TILE_EDIT                            (WM_USER + 5)
#define WM_VIEW_ACTIVE                          (WM_USER + 6)
#define WM_VIEW_CLOSE                           (WM_USER + 7)

#define PALETTE_FOCUS_COLOR_IN                  (RGB(255, 255, 0))
#define PALETTE_FOCUS_COLOR_MID                 (RGB(255, 255, 0))
#define PALETTE_FOCUS_COLOR_OUT                 (RGB(255, 0, 0))

#define PALETTE_GROUP_FOCUS_COLOR_IN            (RGB(0, 0, 0))
#define PALETTE_GROUP_FOCUS_COLOR_MID           (RGB(255, 255, 0))
#define PALETTE_GROUP_FOCUS_COLOR_OUT           (RGB(255, 255, 0))

#define TILE_FOCUS_COLOR_IN                     (RGB(255, 0, 0))
#define TILE_FOCUS_COLOR_MID                    (RGB(255, 255, 0))
#define TILE_FOCUS_COLOR_OUT                    (RGB(255, 255, 0))
#define TILE_HOVER_COLOR                        (RGB(255, 255, 0))
#define TILE_CHECKED_COLOR                      (RGB(255, 0, 0))

#define TILE_GRID_LINE_COLOR                    (RGB(255, 255, 0))
#define TILE_TILE_LINE_COLOR                    (RGB(128, 128, 128))

#define TILE_EDIT_LINE_COLOR                    (RGB(255, 255, 128))

//BMP图片使用
#pragma pack (1)
typedef struct _IMAGE_BMP
{
    BITMAPFILEHEADER bitmapHeader;
    BITMAPINFOHEADER bitmapInfo;

    _IMAGE_BMP()
    {
        ZeroMemory(this, sizeof(_IMAGE_BMP));
    }
}BMP_FORMAT_INFO, * PIMAGE_BMP;
#pragma pack()

typedef struct _TILE_PIXEL
{
    uint8_t color[8][8] = { 0 };        //像素颜色索引数组
}TILE_PIXEL;

extern DWORD g_PaletteColorRGB[0x40];
extern DWORD g_PaletteColorBGR[0x40];