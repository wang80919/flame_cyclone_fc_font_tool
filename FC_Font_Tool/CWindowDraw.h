#pragma once
#include "CWindow/CWindowBase.h"
#include <map>
#include <vector>
#include "CBitmapDraw.h"
#include "Common.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <set>

#define EXPORT_BUF_SIZE             (1024 * 1024 * 32)
#define OUT_DC_WIDTH                (256)
#define OUT_DC_HEIGHT               (128 * 8)

enum eChrOutMode
{
    eOutNormal,                 // 普通
    eOutHorizontal,             // 水平
    eOutVertical,               // 垂直
    eOutHorizontalCompress,     // 水平压缩
    eOutVerticalCompress        // 垂直压缩
};

typedef struct _TEXT_CODE_Y_X
{
    std::map<int, std::set<uint8_t>> mapData;
}TEXT_CODE_Y_X;

typedef struct _TEXT_TILE_ID
{
    uint8_t x;
    uint8_t y;
}TEXT_TILE_ID;

class CWindowDraw: public CWindowBase
{
public:

    void SetText(const _tstring& str);
    _tstring GetCode(const _tstring& str);
    void SetFont(const LOGFONT* lplf);
    void SetOutMode(eChrOutMode eMode);
    void UpdateAsync();
    void UpdateSync();
    void UpdateText();
    bool ExportChr(
        const _tstring strPath, 
        std::function<bool(
            double progress, 
            size_t cur, 
            size_t total, 
            double speed, 
            double costTime, 
            double remainingTime
            )> cbProcess = nullptr
    );

    int GetScrollPos();
    void SetScrollPos(int nPos);
    const LOGFONT& GetFontInfo() const;
    int GetCharCount() const;
    void SetOnTextChange(std::function<void(int nCount, int nOutSize)> cb);
    void SetOnPaintFinish(std::function<void()> cb);
    bool WaitTaskQuit();

public:
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    LRESULT OnEraseBkgnd(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
    LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);

protected:

    DECLARE_WND_MESSAGE_MAP()

private:

    void _PaintTask();
    void _PaintText();
    _tstring _GetCode(const _tstring& str);
    _tstring _GetCodeEx(const _tstring& str);
    void _UpdateScroll();
    void _PaintGrid();
    void _UpdateText(const _tstring& str);
    bool _CreateOutFontBitmap();
    bool _CreateBufFontBitmap();
    void _OutByMode(const _tstring& str, int nPosY);

public:

    BYTE m_Palette[0x10] =
    {
        0x0F,0x2A,0x12,0x2C,0x0F,0x21,0x21,0x21,0x0F,0x27,0x22,0x22,0x0F,0x22,0x22,0x22
    };

    DWORD           m_nBgPalIndex = 0;          // 背景颜色
    DWORD           m_nChPalIndex = 1;          // 文本颜色
    DWORD           m_nShadePalIndex = 2;       // 阴影颜色
    std::function<void(int nCount, int nTotal)> m_cbTextChange = nullptr;
    std::function<void()> m_cbPaintFinish = nullptr;

    LOGFONT         m_lf;                       // 逻辑字体信息
    double          m_fontSize = 11;            // 字体大小
    int             m_fontWidth = 16;           // 字体宽度
    int             m_fontOutWidth = 128;        // 输出字体宽度
    int             m_fontHeight = 16;          // 字体高度
    int             m_fontOrientation = 0;      // 旋转角度
    int             m_offsetX = 0;              // 水平偏移
    int             m_offsetY = 0;              // 垂直偏移
    int             m_shadeX = 0;               // 水平偏移
    int             m_shadeY = 0;               // 垂直偏移
    int             m_fontStart = 0;

    bool            m_fStartChar = false;       // 启用起始字符 
    bool            m_fCodeComment = false;     // 码表前输出居中偏移
    bool            m_fCodeCenter = false;      // 码表前输出居中偏移
    bool            m_fCodeSize = false;        // 码表前输出长度
    bool            m_fCodeMask = false;        // 码表长度 设置最高位
    bool            m_fCodeX2 = false;          // 码表字符X输出2个字节
    bool            m_fCodeY2 = true;           // 码表字符Y输出2个字节
    bool            m_fShade = false;           // 显示阴影
    bool            m_fGrid = true;             // 是否显示网格
    bool            m_fQuit = false;            // 退出
    bool            m_fNoReturn = false;        // 是否忽略转行
    bool            m_fNoSpace = false;         // 是否忽略空格
    bool            m_fNoDuplicate = false;     // 去除重复字符
    bool            m_fAsciiFull = false;       // 英文字符全宽
    bool            m_fontBold = false;         // 粗体
    bool            m_fontItanlic = false;      // 斜体
    bool            m_fontUnderLine = false;    // 下划线
    bool            m_fontStrikeOut = false;    // 删除线
    bool            m_fDebug = 0;               // 调试
    eChrOutMode     m_eOutMode = eChrOutMode::eOutNormal;

private:

    _tstring        m_strText;                  // 文本内容
    _tstring        m_strCode;                  // 码表内容
    std::vector<_tstring> m_vTextList;          // 文本
    std::map<_TCHAR, TEXT_CODE_Y_X> m_mapCode;  // 码表
    std::map<_tstring, std::vector<TEXT_TILE_ID>>    m_vecCode;            //码表

    HFONT           m_hFont = NULL;             // 逻辑字体
    HDC             m_hOutFontDC = NULL;        // 输出DC
    HBITMAP         m_hOutFontBitmap = NULL;    // 输出位图 128 * 128
    DWORD*          m_pOutFontPixels = NULL;    // 输出位图数据

    HDC             m_hBufFontDC = NULL;        // 输出DC
    HBITMAP         m_hBufFontBitmap = NULL;    // 输出位图 128 * 128
    DWORD*          m_pBufFontPixels = NULL;    // 输出位图数据

    HDC             m_hBackDC = NULL;           // 后台DC
    HBITMAP         m_hBackBitmap = NULL;       // 后台位图

    HDC             m_hMemDC = NULL;            // 内存DC
    HBITMAP         m_hMemBitmap = NULL;        // 内存位图

    std::thread     m_paintTask;                // 绘图任务
    std::condition_variable m_cv;               // 绘图条件变量
    std::mutex      m_lock;                     // 绘图锁
    std::mutex      m_mCode;                    // 码表锁
};

