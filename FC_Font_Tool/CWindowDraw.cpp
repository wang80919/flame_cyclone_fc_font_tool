#include "CWindowDraw.h"
#include <windowsx.h>
#include "Win32Utils/CStrUtils.h"
#include "Win32Utils/CBytesUtils.h"
#include "Win32Utils/CFileUtils.h"

BEGIN_WND_MESSAGE_MAP(CWindowDraw, CWindowBase)
    ON_WND_MESSAGE(WM_CREATE, &CWindowDraw::OnCreate)
    ON_WND_MESSAGE(WM_CLOSE, &CWindowDraw::OnClose)
    ON_WND_MESSAGE(WM_DESTROY, &CWindowDraw::OnDestroy)
    ON_WND_MESSAGE(WM_ERASEBKGND, &CWindowDraw::OnEraseBkgnd)
    ON_WND_MESSAGE(WM_PAINT, &CWindowDraw::OnPaint)
    ON_WND_MESSAGE(WM_VSCROLL, &CWindowDraw::OnVScroll)
    ON_WND_MESSAGE(WM_MOUSEWHEEL, &CWindowDraw::OnMouseWheel)
END_WND_MESSAGE_MAP()

DWORD g_PalColor[4] = {
    RGB(8, 8, 8),
    RGB(99, 207, 99),
    RGB(57, 51, 255),
    RGB(220, 255, 255)
};

LRESULT CWindowDraw::OnCreate(WPARAM wParam, LPARAM lParam)
{
    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);
    WORD cxClient = rect.right - rect.left;
    WORD cyClient = rect.bottom - rect.top;

    HDC hdc = ::GetDC(m_hWnd);

    BuildDcBitmap(hdc, &m_hMemDC, &m_hMemBitmap, cxClient, cyClient);
    BuildDcBitmap(hdc, &m_hBackDC, &m_hBackBitmap, cxClient, cyClient);

    ::ReleaseDC(m_hWnd, hdc);

    m_paintTask = std::move(std::thread([this]() {

        _PaintTask();

        }
    )
    );

    m_paintTask.detach();

    _CreateOutFontBitmap();
    _CreateBufFontBitmap();
    return FALSE;
}

LRESULT CWindowDraw::OnClose(WPARAM wParam, LPARAM lParam)
{
    WaitTaskQuit();

    {
        DeleteDcBitmap(&m_hMemDC, &m_hMemBitmap);
        DeleteDcBitmap(&m_hBackDC, &m_hBackBitmap);
    }

    return FALSE;
}

bool CWindowDraw::WaitTaskQuit()
{
    m_fQuit = true;
    if (m_paintTask.joinable())
    {
        m_cv.notify_one();
        m_paintTask.join();
    }

    return true;
}

LRESULT CWindowDraw::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

LRESULT CWindowDraw::OnEraseBkgnd(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

LRESULT CWindowDraw::OnPaint(WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = { 0 };
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);

    {
        std::lock_guard <std::mutex > lock(m_lock);
        ::BitBlt(hdc, 0, 0, rect.right, rect.bottom, m_hMemDC, 0, 0, SRCCOPY);
    }

    ::EndPaint(m_hWnd, &ps);

    return FALSE;
}

LRESULT CWindowDraw::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

    SCROLLINFO si = { 0 };
    int nBar = SB_VERT;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(m_hWnd, nBar, &si);

    if (zDelta > 0)
    {
        if (si.nPos > 0)
        {
            si.nPos -= 1;
        }

        if (si.nPos < 0)
        {
            si.nPos = 0;
        }
    }
    else
    {
        if ((si.nPos + 1) < si.nMax)
        {
            si.nPos += 1;
        }

        if ((si.nPos + 1) > si.nMax)
        {
            si.nPos = si.nMax - 1;
        }
    }

    ::SetScrollInfo(m_hWnd, nBar, &si, true);
    UpdateAsync();
    return FALSE;
}

LRESULT CWindowDraw::OnVScroll(WPARAM wParam, LPARAM lParam)
{
    SCROLLINFO si = { 0 };
    int nBar = SB_VERT;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    ::GetScrollInfo(m_hWnd, nBar, &si);

    WORD wEvent = LOWORD(wParam);
    switch (wEvent)
    {
    case SB_ENDSCROLL:
    {
    }
    break;
    case SB_TOP:
    {
        si.nPos = 0;
    }
    break;
    case SB_BOTTOM:
    {
        si.nPos = si.nMax;
    }
    break;
    case SB_LINEUP:
    {
        if (si.nPos > 0)
        {
            si.nPos -= 1;
        }
    }
    break;
    case SB_LINEDOWN:
    {
        if ((si.nPos + 1) < si.nMax)
        {
            si.nPos += 1;
        }
    }
    break;
    case SB_PAGEUP:
    {
        if (si.nPos < si.nPage)
        {
            si.nPos = 0;
        }
        else
        {
            si.nPos -= si.nPage;
        }
    }
    break;
    case SB_PAGEDOWN:
    {
        if ((si.nPos + si.nPage) > (si.nMax - si.nPage))
        {
            si.nPos = si.nMax - si.nPage;
        }
        else
        {
            si.nPos += si.nPage;
        }
    }
    break;
    case SB_THUMBPOSITION:
    {
        // 拖动操作结束时滚动框的位置
        //WORD wPos = HIWORD(wParam);

        si.nPos = si.nTrackPos;
    }
    break;
    case SB_THUMBTRACK:
    {
        // 滚动框拖动到的位置
        //WORD wPos = HIWORD(wParam);

        si.nPos = si.nTrackPos;
    }
    break;
    }

    if (si.nPos + 1 > si.nMax)
    {
        si.nPos = si.nMax - 1;
    }

    ::SetScrollInfo(m_hWnd, nBar, &si, true);

    //重绘
    UpdateAsync();

    return FALSE;
}

void CWindowDraw::SetText(const _tstring& str)
{
    _UpdateText(str);
}

_tstring CWindowDraw::GetCode(const _tstring& str)
{
    if (m_fNoDuplicate)
    {
        return _GetCode(str);
    }

    return _GetCodeEx(str);
}

int CWindowDraw::GetScrollPos()
{
    SCROLLINFO si = { 0 };
    int nBar = SB_VERT;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    ::GetScrollInfo(m_hWnd, nBar, &si);

    return si.nPos;
}

void CWindowDraw::SetScrollPos(int nPos)
{
    SCROLLINFO si = { 0 };
    int nBar = SB_VERT;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    ::GetScrollInfo(m_hWnd, nBar, &si);
    si.nPos = nPos;

    ::SetScrollInfo(m_hWnd, nBar, &si, true);
}

const LOGFONT& CWindowDraw::GetFontInfo() const
{
    return m_lf;
}

int CWindowDraw::GetCharCount() const
{
    int nChCount = 0;
    for (const auto& item : m_vTextList)
    {

        nChCount += item.size();
    }

    return nChCount;
}

void CWindowDraw::SetOnTextChange(std::function<void(int nCount, int nOutSize)> cb)
{
    m_cbTextChange = cb;
}

void CWindowDraw::SetOnPaintFinish(std::function<void()> cb)
{
    m_cbPaintFinish = cb;
}

bool CWindowDraw::_CreateOutFontBitmap()
{
    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);

    HDC hdc = ::GetDC(NULL);
    m_hOutFontDC = ::CreateCompatibleDC(hdc);
    ::ReleaseDC(NULL, hdc);

    HBITMAP hBitmap;
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = OUT_DC_WIDTH;
    bmi.bmiHeader.biHeight = -OUT_DC_HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    hBitmap = ::CreateDIBSection(m_hOutFontDC, &bmi, DIB_RGB_COLORS, (void**)&m_pOutFontPixels, NULL, 0);

    if (NULL == hBitmap)
    {
        return false;
    }

    if(m_hOutFontBitmap != NULL)
    {
        ::DeleteObject(m_hOutFontBitmap);
    }

    ::SelectObject(m_hOutFontDC, hBitmap);
    m_hOutFontBitmap = hBitmap;

    return true;
}

bool CWindowDraw::_CreateBufFontBitmap()
{
    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);

    HDC hdc = ::GetDC(NULL);
    m_hBufFontDC = ::CreateCompatibleDC(hdc);
    ::ReleaseDC(NULL, hdc);

    HBITMAP hBitmap;
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = OUT_DC_WIDTH;
    bmi.bmiHeader.biHeight = -OUT_DC_HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    hBitmap = ::CreateDIBSection(m_hBufFontDC, &bmi, DIB_RGB_COLORS, (void**)&m_pBufFontPixels, NULL, 0);

    if (NULL == hBitmap)
    {
        return false;
    }

    if(m_hBufFontBitmap != NULL)
    {
        ::DeleteObject(m_hBufFontBitmap);
    }

    ::SelectObject(m_hBufFontDC, hBitmap);
    m_hBufFontBitmap = hBitmap;

    return true;
}

void CWindowDraw::SetFont(const LOGFONT* lplf)
{
    m_lf = *lplf;

    LOGFONT lfTmp = m_lf;
    lfTmp.lfEscapement = m_fontOrientation * 10;
    lfTmp.lfOrientation = m_fontOrientation * 10;

    lfTmp.lfHeight = (m_fontSize * (double)::GetDeviceCaps(m_hOutFontDC, LOGPIXELSY)) / 72.0f;

    HFONT hFont = ::CreateFontIndirect(&lfTmp);

    if (hFont)
    {
        ::SelectObject(m_hOutFontDC, hFont);
    }

    if (NULL != m_hFont)
    {
        ::DeleteObject(m_hFont);
    }

    m_hFont = hFont;

    UpdateText();
}

void CWindowDraw::SetOutMode(eChrOutMode eMode)
{
    m_eOutMode = eMode;

    _UpdateText(m_strText);
    UpdateAsync();
}

void CWindowDraw::UpdateAsync()
{
    m_cv.notify_one();
}

void CWindowDraw::UpdateText()
{
    _UpdateText(m_strText);
    UpdateAsync();
}

void CWindowDraw::UpdateSync()
{
    static std::mutex mt;

    std::lock_guard <std::mutex > lockUpdate(mt);
    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);

    _PaintText();

    // 绘制网格
    if (m_fGrid)
    {
        _PaintGrid();
    }

    {
        std::lock_guard <std::mutex > lock(m_lock);
        ::BitBlt(m_hMemDC, 0, 0, rect.right, rect.bottom, m_hBackDC, 0, 0, SRCCOPY);
    }
    ::InvalidateRect(m_hWnd, NULL, FALSE);
}

void CWindowDraw::_PaintTask()
{
    ::GdiFlush();
    std::mutex mt;
    std::unique_lock<std::mutex> lock(mt);
    while (!m_fQuit)
    {
        m_cv.wait(lock);
        UpdateSync();
    }
}

void CWindowDraw::_UpdateText(const _tstring& str)
{
    m_strText = str;
    m_vTextList.clear();

    TCHAR szBuf[MAX_PATH] = { 0 };
    int nCount = 0;
    int nWidth = 0;
    int nCharWidth = 0;
    bool fManualReturn = false;

    int nTextWidth = 128;
    if (eChrOutMode::eOutHorizontalCompress == m_eOutMode)
    {
        nTextWidth = 256;
    }

    std::set<TCHAR> setChar;

    if (m_fStartChar)
    {
        for (int i = 0; i < m_fontStart; i++)
        {
            TCHAR item = _T(' ');
            if (item > 0x7F)
            {
                nCharWidth = m_fontWidth;
            }
            else
            {
                if (m_fAsciiFull)
                {
                    nCharWidth = m_fontWidth;
                }
                else
                {
                    nCharWidth = m_fontWidth / 2;
                }
            }

            if (nCharWidth < 1)
            {
                nCharWidth = 1;
            }

            if (nWidth + nCharWidth > nTextWidth)
            {
                szBuf[nCount] = _T('\0');
                m_vTextList.push_back(szBuf);
                nCount = 0;
                nWidth = 0;
                fManualReturn = true;
                continue;
            }

            szBuf[nCount++] = item;
            szBuf[nCount] = _T('\0');
            fManualReturn = false;

            nWidth += nCharWidth;

            if (nTextWidth == nWidth)
            {
                m_vTextList.push_back(szBuf);
                nCount = 0;
                nWidth = 0;
                fManualReturn = true;
            }
        }
    }

    for (const auto& item : m_strText)
    {
        if (item > 0x7F)
        {
            nCharWidth = m_fontWidth;
        }
        else
        {
            if (m_fAsciiFull)
            {
                nCharWidth = m_fontWidth;
            }
            else
            {
                nCharWidth = m_fontWidth / 2;
            }
        }

        if (nCharWidth < 1)
        {
            nCharWidth = 1;
        }

        if (m_fNoReturn || m_fNoSpace)
        {
            if (m_fNoSpace && item == _T(' '))
            {
                continue;
            }

            if (item == _T('\r') || item == _T('\n'))
            {
                continue;
            }

            if (m_fNoDuplicate)
            {
                auto itResult = setChar.insert(item);
                if (!itResult.second)
                {
                    continue;
                }
            }

            if (nWidth + nCharWidth > nTextWidth)
            {
                szBuf[nCount] = _T('\0');
                m_vTextList.push_back(szBuf);
                nCount = 0;
                nWidth = 0;
            }

            szBuf[nCount++] = item;
            szBuf[nCount] = _T('\0');

            nWidth += nCharWidth;

            if (nTextWidth == nWidth)
            {
                m_vTextList.push_back(szBuf);
                nCount = 0;
                nWidth = 0;
            }
        }
        else
        {
            if (m_fNoSpace && item == _T(' '))
            {
                continue;
            }

            if (item == _T('\r'))
            {
                continue;
            }

            if (item == _T('\n'))
            {
                szBuf[nCount] = _T('\0');
                if (!fManualReturn)
                {
                    m_vTextList.push_back(szBuf);
                }
                nCount = 0;
                nWidth = 0;
                fManualReturn = false;
                continue;
            }

            if (m_fNoDuplicate)
            {
                auto itResult = setChar.insert(item);
                if (!itResult.second)
                {
                    continue;
                }
            }

            if (nWidth + nCharWidth > nTextWidth)
            {
                szBuf[nCount] = _T('\0');
                m_vTextList.push_back(szBuf);
                nCount = 0;
                nWidth = 0;
                fManualReturn = true;
                continue;
            }

            szBuf[nCount++] = item;
            szBuf[nCount] = _T('\0');
            fManualReturn = false;

            nWidth += nCharWidth;

            if (nTextWidth == nWidth)
            {
                m_vTextList.push_back(szBuf);
                nCount = 0;
                nWidth = 0;
                fManualReturn = true;
            }
        }
    }

    if (nCount > 0)
    {
        m_vTextList.push_back(szBuf);
        nCount = 0;
    }

    // 去除最后的空行
    if (!m_vTextList.empty())
    {
        int nIndex = m_vTextList.size();
        for (int i = m_vTextList.size() - 1; i >= 0; i--)
        {
            if (m_vTextList[i] != _T(""))
            {
                break;
            }

            nIndex--;
        }

        m_vTextList.resize(nIndex);
    }

    _UpdateScroll();
    UpdateSync();
}

void CWindowDraw::_UpdateScroll()
{
    SCROLLINFO si = { 0 };
    int nBar = SB_VERT;
    si.cbSize = sizeof(SCROLLINFO);
    si.nMin = 0;
    si.fMask = SIF_ALL;
    ::GetScrollInfo(m_hWnd, nBar, &si);
    si.nPage = 14;

    si.nMax = (m_vTextList.size() * m_fontHeight) / 8;
    if (0 != (m_vTextList.size() * m_fontHeight) % 8)
    {
        si.nMax += 1;
    }

    if (si.nPos + 1 > si.nMax)
    {
        si.nPos = si.nMax;
    }

    ::SetScrollInfo(m_hWnd, nBar, &si, true);


    if (m_cbTextChange)
    {
        int nTileHeight = m_vTextList.size() * m_fontHeight;
        int nVTileCount = nTileHeight / 8;
        if (0 != nTileHeight % 8)
        {
            nVTileCount = nTileHeight / 8 + 1;
        }

        int nSize = nVTileCount * 16 * 16;
        m_cbTextChange(GetCharCount(), nSize);
    }
}

void CWindowDraw::_PaintGrid()
{
    HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
    HPEN hOldPen = (HPEN)::SelectObject(m_hBackDC, hPen);
    int nMultiple = 4;
    int nLineCount = 16;
    if (m_fDebug)
    {
        nMultiple = 3;

        // 横线
        for (int i = 0; i < nLineCount; i++)
        {
            MoveToEx(m_hBackDC, 0, i * 8 * nMultiple, nullptr);
            LineTo(m_hBackDC, 384, i * 8 * nMultiple);
        }

        // 竖线
        for (int i = 0; i < nLineCount; i++)
        {
            MoveToEx(m_hBackDC, i * 8 * nMultiple, 0, nullptr);
            LineTo(m_hBackDC, i * 8 * nMultiple, 384);
        }
    }
    else
    {
        for (int i = 0; i < nLineCount; i++)
        {
            MoveToEx(m_hBackDC, 0, i * 8 * nMultiple, nullptr);
            LineTo(m_hBackDC, 512, i * 8 * nMultiple);
        }

        for (int i = 0; i < nLineCount; i++)
        {
            MoveToEx(m_hBackDC, i * 8 * nMultiple, 0, nullptr);
            LineTo(m_hBackDC, i * 8 * nMultiple, 512);
        }
    }
    (HPEN)::SelectObject(m_hBackDC, hOldPen);
    ::DeleteObject(hPen);
}

void CWindowDraw::_PaintText()
{
    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);

    ::SetBkMode(m_hOutFontDC, TRANSPARENT);
    ::SetBkColor(m_hOutFontDC, RGB(8, 8, 8));
    ::SetTextColor(m_hOutFontDC, g_PalColor[m_nChPalIndex]);

    RECT rc = { 0, 0, 256, 512 };

    // 背景色
    HBRUSH hBrush = ::CreateSolidBrush(g_PalColor[m_nBgPalIndex]);
    ::FillRect(m_hBackDC, &rect, hBrush);
    ::FillRect(m_hOutFontDC, &rc, hBrush);
    ::DeleteObject(hBrush);

    // 从滚动条指定位置开始绘制
    int nPosY = 0;
    int nPosEndY = 0;
    int nScrollPos = GetScrollPos();
    int nDrawPos = nScrollPos * 8 / m_fontHeight;

    int nFontHeight = ::MulDiv(m_fontSize, ::GetDeviceCaps(m_hOutFontDC, LOGPIXELSY), 72);
    int nStartLine = nFontHeight / m_fontHeight;

    // 从滚动条前8开始绘制
    if (nDrawPos > (nFontHeight / 8))
    {
        nDrawPos -= (nFontHeight / 8);
        nPosY -= (nFontHeight / 8) * m_fontHeight;
    }
    else
    {
        nPosY -= nDrawPos * m_fontHeight;
        nDrawPos -= nDrawPos;
    }

    std::lock_guard <std::mutex > lock(m_mCode);
    m_mapCode.clear();
    m_vecCode.clear();
    int nStartY = nPosY;

    for (int i = nDrawPos; i < m_vTextList.size() && nPosY < OUT_DC_HEIGHT; i++)
    {
        _tstring strItem = m_vTextList[i];
        std::vector<TEXT_TILE_ID> vCodeYX;

        // 绘制阴影
        if (m_fShade && (m_eOutMode < eChrOutMode::eOutVerticalCompress))
        {
            ::SetTextColor(m_hOutFontDC, g_PalColor[m_nShadePalIndex]);
            int nPosX = 0;
            for (int j = 0; j < strItem.size(); j++)
            {
                ::TextOut(m_hOutFontDC, nPosX + m_shadeX + m_offsetX + 1, nPosY + m_shadeY + m_offsetY + 1, &strItem[j], 1);
                if (strItem[j] > 0x80)
                {
                    nPosX += m_fontWidth;
                }
                else
                {
                    nPosX += (strItem[j] < 0x80 && !m_fAsciiFull) ? (m_fontWidth / 2) : m_fontWidth;
                }
            }
        }

        // 绘制文本
        {
            ::SetTextColor(m_hOutFontDC, g_PalColor[m_nChPalIndex]);
            int nPosX = 0;
            for (int j = 0; j < strItem.size(); j++)
            {
                ::TextOut(m_hOutFontDC, nPosX + m_offsetX, nPosY + m_offsetY, &strItem[j], 1);
                int nFontWidth = m_fontWidth;
                if (strItem[j] < 0x80 && !m_fAsciiFull)
                {
                    nFontWidth = m_fontWidth / 2;
                }

                TEXT_TILE_ID code;
                code.x = nPosX;
                code.y = nPosY;
                vCodeYX.push_back(code);

                auto itText = m_mapCode.find(strItem[j]);
                int keyY = nPosY - nStartY;
                if (itText == m_mapCode.end())
                {
                    TEXT_CODE_Y_X code;
                    std::set<uint8_t> element = { (uint8_t)nPosX };
                    auto& data = code.mapData;
                    data.insert(std::make_pair(keyY, element));
                    m_mapCode.insert(std::make_pair(strItem[j], code));
                }
                else
                {
                    auto& data = itText->second.mapData;
                    auto itInfo = data.find(keyY);
                    if (itInfo == data.end())
                    {
                        std::set<uint8_t> element = { (uint8_t)nPosX };
                        data.insert(std::make_pair(keyY, element));
                    }
                    else
                    {
                        itInfo->second.insert(nPosX);
                    }
                }

                nPosX += nFontWidth;
            }

            m_vecCode.insert(std::make_pair(strItem, vCodeYX));
        }

        _OutByMode(strItem, nPosY);

        if (eChrOutMode::eOutVerticalCompress == m_eOutMode)
        {
            nPosY += m_fontHeight / 2;
        }
        else
        {
            nPosY += m_fontHeight;
        }

        // 绘制结束标记
        if (i == m_vTextList.size() - 1)
        {
            nPosEndY = nPosY;
        }
    }

    if (m_vTextList.empty())
    {
        HBRUSH hBrush = ::CreateHatchBrush(HS_BDIAGONAL, RGB(128, 128, 128));
        RECT rect = { 0, 0, 256, 128};
        ::FillRect(m_hOutFontDC, &rect, hBrush);
        ::DeleteObject(hBrush);
    }

    double fRatio = (double)m_fontOutWidth / (double)m_fontWidth;

    ::SetStretchBltMode(m_hOutFontDC, 0);
    ::SetStretchBltMode(m_hBufFontDC, 0);
    ::SetStretchBltMode(m_hBackDC, 0);

    // 拷贝拉伸到后台DC
    if (m_fDebug)
    {
        ::StretchBlt(m_hBackDC, 0, 0, 384, 384, m_hOutFontDC, 0, (nScrollPos * 8) % m_fontHeight, 128, 128, SRCCOPY);
        ::StretchBlt(m_hBackDC, 384, 0, 128, 512, m_hOutFontDC, 0, 0, 128, 512, SRCCOPY);

        RECT rtDebug = { 384, ((nScrollPos * 8) % m_fontHeight) * 1, 512 - 1, ((nScrollPos * 8) % m_fontHeight + 128) * 1 - 1};
        HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 0, 0));
        ::FrameRect(m_hBackDC, &rtDebug, hBrush);
        ::DeleteObject(hBrush);
    }
    else
    {
        int outWidth = m_fontOutWidth;

        ::StretchBlt(m_hBufFontDC, 0, 0, outWidth, 128, m_hOutFontDC, 0, (nScrollPos * 8) % m_fontHeight, 128, 128, SRCCOPY);
        ::StretchBlt(m_hBackDC, 0, 0, 512, 512, m_hBufFontDC, 0, 0, 128, 128, SRCCOPY);
    }

    // 右边填充
    {
        int outWidth = m_fontOutWidth * 4;
        HBRUSH hBkgBrush = ::CreateSolidBrush(g_PalColor[m_nBgPalIndex]);
        RECT rtFill = { outWidth , 0, 512, 512 };
        ::FillRect(m_hBackDC, &rtFill, hBkgBrush);
        ::DeleteObject(hBkgBrush);
    }

    // 结束
    {
        HBRUSH hBrush = ::CreateHatchBrush(HS_BDIAGONAL, RGB(128, 128, 128));
        RECT rect = { 0, nPosEndY * 4, 512, 512 };
        ::FillRect(m_hBackDC, &rect, hBrush);
        ::DeleteObject(hBrush);
    }

    if (m_cbPaintFinish)
    {
        m_cbPaintFinish();
    }
}

_tstring CWindowDraw::_GetCode(const _tstring& str)
{
    std::vector<_tstring> strInputLines = CStrUtils::SplitLines(str);

    std::lock_guard <std::mutex > lock(m_mCode);
    _tstring strOutText;
    _tstring strLineOut;
    _tstring strOut;
    int nCount = 0;
    for (const auto& line : strInputLines)
    {
        strLineOut.clear();
        strOut.clear();
        nCount = 0;

        if (m_fCodeComment)
        {
            strOut += _T(";");
            strOut += line;
            strOut += _T("\r\n");
        }

        for (const auto& item : line)
        {
            auto it = m_mapCode.find(item);
            if (it != m_mapCode.end())
            {
                auto& data = it->second.mapData;
                for (const auto& into : data)
                {
                    auto& y = into.first;
                    for (const auto& x : into.second)
                    {
                        int tile = y / 8 * 16 + x / 8;
                        tile = tile & 0xFF;
                        strLineOut += CStrUtils::Format(_T("%02X "), tile);
                        nCount++;

                        if (m_fCodeX2)
                        {
                            if (item < 0x80)
                            {
                                if (m_fAsciiFull)
                                {
                                    strLineOut += CStrUtils::Format(_T("%02X "), tile + 1);
                                    nCount++;
                                }
                            }
                            else
                            {
                                strLineOut += CStrUtils::Format(_T("%02X "), tile + 1);
                                nCount++;
                            }
                        }
                    }
                }
            }
            else
            {
                strLineOut += CStrUtils::Format(_T("%02X "), 00);
                nCount++;
            }

        }

        if (m_fCodeCenter)
        {
            int Center = (0x20 - nCount) / 2;
            if (nCount > 0x20)
            {
                Center = 0;
            }

            strOut += CStrUtils::Format(_T("%02X "), Center);
        }

        if (m_fCodeSize)
        {
            strOut += CStrUtils::Format(_T("%02X "), nCount | (m_fCodeMask ? 0x80 : 0x00));
        }

        if (!strLineOut.empty())
        {
            if (_T(' ') == strLineOut.back())
            {
                strLineOut.resize(strLineOut.size() - 1);
            }
        }

        strOut += strLineOut;

        strLineOut.clear();
        if (m_fCodeY2)
        {
            strOut += _T("\r\n");

            for (const auto& item : line)
            {
                auto it = m_mapCode.find(item);
                if (it != m_mapCode.end())
                {
                    auto& data = it->second.mapData;
                    for (const auto& into : data)
                    {
                        auto& y = into.first;
                        for (const auto& x : into.second)
                        {
                            int tile = y / 8 * 16 + x / 8 + 16;
                            tile = tile & 0xFF;
                            strLineOut += CStrUtils::Format(_T("%02X "), tile);

                            if (m_fCodeX2)
                            {
                                if (item < 0x80)
                                {
                                    if (m_fAsciiFull)
                                    {
                                        strLineOut += CStrUtils::Format(_T("%02X "), tile + 1);
                                    }
                                }
                                else
                                {
                                    strLineOut += CStrUtils::Format(_T("%02X "), tile + 1);
                                }
                            }
                        }
                    }
                }
                else
                {
                    strLineOut += CStrUtils::Format(_T("%02X "), 00);
                }
            }
        }

        if (m_fCodeY2)
        {
            if (m_fCodeCenter)
            {
                int Center = (0x20 - nCount) / 2;
                if (nCount > 0x20)
                {
                    Center = 0;
                }

                strOut += CStrUtils::Format(_T("%02X "), Center);
            }

            if (m_fCodeSize)
            {
                strOut += CStrUtils::Format(_T("%02X "), nCount | (m_fCodeMask ? 0x80 : 0x00));
            }
        }

        if (!strLineOut.empty())
        {
            if (_T(' ') == strLineOut.back())
            {
                strLineOut.resize(strLineOut.size() - 1);
            }
        }

        strOut += strLineOut;
        strOutText += strOut;
        strOutText += _T("\r\n");
    }

    return strOutText;
}

_tstring CWindowDraw::_GetCodeEx(const _tstring& str)
{
    std::vector<_tstring> strInputLines = CStrUtils::SplitLines(str);

    std::lock_guard <std::mutex > lock(m_mCode);
    _tstring strOutText;
    _tstring strLineOut;
    _tstring strOut;
    int nCount = 0;

    for (const auto& line : strInputLines)
    {
        std::vector<uint8_t> vTileID;

        strLineOut.clear();
        strOut.clear();
        nCount = 0;

        if (m_fCodeComment)
        {
            strOut += _T(";");
            strOut += line;
            strOut += _T("\r\n");
        }

        for (const auto& code : m_vecCode)
        {
            size_t nPos = code.first.find(line);
            if (_tstring::npos != nPos)
            {
                int nIndex = 0;
                for (const auto& ch : line)
                {
                    int x = code.second[nPos + nIndex].x;
                    int y = code.second[nPos + nIndex].y;

                    int tile = y / 8 * 16 + x / 8;
                    tile = tile & 0xFF;
                    vTileID.push_back(tile);
                    nIndex++;
                }
            }
        }

        size_t nPos = 0;
        for (const auto& item : vTileID)
        {
            strLineOut += CStrUtils::Format(_T("%02X "), item);

            if (m_fCodeX2)
            {
                if (line[nPos] < 0x80)
                {
                    if (m_fAsciiFull)
                    {
                        strLineOut += CStrUtils::Format(_T("%02X "), item + 1);
                        nCount++;
                    }
                }
                else
                {
                    strLineOut += CStrUtils::Format(_T("%02X "), item + 1);
                    nCount++;
                }
            }
            nCount++;
            nPos++;
        }

        if (m_fCodeCenter)
        {
            int Center = (0x20 - nCount) / 2;
            if (nCount > 0x20)
            {
                Center = 0;
            }

            strOut += CStrUtils::Format(_T("%02X "), Center);
        }

        if (m_fCodeSize)
        {
            strOut += CStrUtils::Format(_T("%02X "), nCount | (m_fCodeMask ? 0x80 : 0x00));
        }

        if (!strLineOut.empty())
        {
            if (_T(' ') == strLineOut.back())
            {
                strLineOut.resize(strLineOut.size() - 1);
            }
        }

        strOut += strLineOut;

        strLineOut.clear();
        if (m_fCodeY2)
        {
            strOut += _T("\r\n");
            nPos = 0;
            for (const auto& item : vTileID)
            {
                strLineOut += CStrUtils::Format(_T("%02X "), item + 16);

                if (m_fCodeX2)
                {
                    if (line[nPos] < 0x80)
                    {
                        if (m_fAsciiFull)
                        {
                            strLineOut += CStrUtils::Format(_T("%02X "), item + 1 + 16);
                        }
                    }
                    else
                    {
                        strLineOut += CStrUtils::Format(_T("%02X "), item + 1 + 16);
                    }
                }
                nPos++;
            }
        }

        if (m_fCodeY2)
        {
            if (m_fCodeCenter)
            {
                int Center = (0x20 - nCount) / 2;
                if (nCount > 0x20)
                {
                    Center = 0;
                }

                strOut += CStrUtils::Format(_T("%02X "), Center);
            }

            if (m_fCodeSize)
            {
                strOut += CStrUtils::Format(_T("%02X "), nCount | (m_fCodeMask ? 0x80 : 0x00));
            }
        }

        if (!strLineOut.empty())
        {
            if (_T(' ') == strLineOut.back())
            {
                strLineOut.resize(strLineOut.size() - 1);
            }
        }

        strOut += strLineOut;
        strOutText += strOut;
        strOutText += _T("\r\n");
    }

    return strOutText;
}

bool CWindowDraw::ExportChr(
    const _tstring strPath,
    std::function<bool(
        double progress,
        size_t cur,
        size_t total,
        double speed,
        double costTime,
        double remainingTime
        )> cbProcess/* = nullptr*/
)
{
    std::vector<uint8_t> vChrData;

    CFileHelper fileHelper;
    if (!fileHelper.Open(strPath))
    {
        return false;
    }

    int nPosY = 0;
    RECT rc = { 0, 0, 128, 256 };

    clock_t beginClock = clock();
    clock_t lastClock = clock();
    clock_t lastSpeedClock = clock();
    clock_t curClock = clock();
    size_t lastSpeedCount = 0;

    int countTotal = GetCharCount();
    int countCur = 0;

    int nTotalLine = m_vTextList.size();
    vChrData.reserve(EXPORT_BUF_SIZE);

    ::SetBkMode(m_hOutFontDC, TRANSPARENT);
    ::SetBkColor(m_hOutFontDC, RGB(0, 0, 0));

    DWORD dwBgColor = g_PalColor[m_nBgPalIndex];
    DWORD dwChColor = g_PalColor[m_nChPalIndex];
    DWORD dwShadeColor = g_PalColor[m_nShadePalIndex];

    // 背景刷子
    HBRUSH hBgBrush = ::CreateSolidBrush(dwBgColor);

    dwBgColor = COLOR_RGB_TO_BGR(g_PalColor[m_nBgPalIndex]);
    dwChColor = COLOR_RGB_TO_BGR(g_PalColor[m_nChPalIndex]);
    dwShadeColor = COLOR_RGB_TO_BGR(g_PalColor[m_nShadePalIndex]);

    int nBufferSize = 0;
    double fRatio = (double)m_fontOutWidth / (double)m_fontWidth;

    for (int i = 0; i < m_vTextList.size(); i++)
    {
        // 输出单行文本
        ::FillRect(m_hOutFontDC, &rc, hBgBrush);
        _tstring strItem = m_vTextList[i];

        // 绘制文本
        int nTmp = nPosY;
        for(int k = 0; k < 1 && (i + k) < m_vTextList.size(); k++)
        {
            _tstring strItem = m_vTextList[i + k];

            // 绘制阴影
            if (m_fShade && (m_eOutMode < eChrOutMode::eOutVerticalCompress))
            {
                ::SetTextColor(m_hOutFontDC, g_PalColor[m_nShadePalIndex]);
                int nPosX = 0;
                for (int j = 0; j < strItem.size(); j++)
                {
                    ::TextOut(m_hOutFontDC, nPosX + m_shadeX + 1, nTmp + m_shadeY + 1, &strItem[j], 1);
                    if (strItem[j] > 0x80)
                    {
                        nPosX += m_fontWidth;
                    }
                    else
                    {
                        nPosX += (strItem[j] < 0x80 && !m_fAsciiFull) ? (m_fontWidth / 2) : m_fontWidth;
                    }
                }
            }

            // 绘制文本
            {
                ::SetTextColor(m_hOutFontDC, g_PalColor[m_nChPalIndex]);
                int nPosX = 0;
                for (int j = 0; j < strItem.size(); j++)
                {
                    ::TextOut(m_hOutFontDC, nPosX + m_offsetX, nTmp + m_offsetY, &strItem[j], 1);
                    if (strItem[j] > 0x80)
                    {
                        nPosX += m_fontWidth;
                    }
                    else
                    {
                        nPosX += (strItem[j] < 0x80 && !m_fAsciiFull) ? (m_fontWidth / 2) : m_fontWidth;
                    }
                }
            }

            _OutByMode(strItem, nTmp);
            nTmp += m_fontHeight;
        }

        int nHeight = m_fontHeight;
        if (eChrOutMode::eOutVerticalCompress == m_eOutMode)
        {
            nHeight = m_fontHeight / 2;
        }

        int outWidth = m_fontOutWidth;

        // 单行拉伸
        {
            RECT rtFill = { outWidth , nPosY, 128, nPosY + nHeight };
            ::FillRect(m_hBackDC, &rtFill, GetStockBrush(BLACK_BRUSH));

            ::SetStretchBltMode(m_hOutFontDC, 0);
            ::SetStretchBltMode(m_hBackDC, 0);
            ::StretchBlt(m_hBackDC, 0, nPosY, outWidth, nHeight, m_hOutFontDC, 0, nPosY, 128, nHeight, SRCCOPY);
            ::BitBlt(m_hOutFontDC, 0, nPosY, outWidth, nHeight, m_hBackDC, 0, nPosY, SRCCOPY);

            rtFill = { outWidth , nPosY, 128, nPosY + nHeight };
            ::FillRect(m_hOutFontDC, &rtFill, hBgBrush);
        }

        nPosY += nHeight;

        //绘制完2行则保存一行
        int nRowCount = ((double)nPosY / (double)8);
        if (nRowCount < 1)
        {
            nRowCount = 1;
        }

        if (i == m_vTextList.size() - 1 && 0 != (nPosY % 8))
        {
            nRowCount += 1;
        }

        for (int row = 0; row < nRowCount; row++)
        {
            for (int col = 0; col < 16; col++)
            {
                TILE_PIXEL tilePixel = { 0 };

                if (m_eOutMode < eChrOutMode::eOutHorizontalCompress)
                {
                    for (int y = 0; y < 8; y++)
                    {
                        for (int x = 0; x < 8; x++)
                        {
                            int posX = col * 8 + x;
                            int posY = row * 8 + y;
                            DWORD dwPos = posY * OUT_DC_WIDTH + posX;

                            tilePixel.color[y][x] = m_nChPalIndex;

                            if (m_pOutFontPixels[dwPos] == dwBgColor)
                            {
                                tilePixel.color[y][x] = m_nBgPalIndex;
                            }
                            else if (m_pOutFontPixels[dwPos] == dwChColor)
                            {
                                tilePixel.color[y][x] = m_nChPalIndex;
                            }
                            else if (m_fShade && m_pOutFontPixels[dwPos] == dwShadeColor)
                            {
                                tilePixel.color[y][x] = m_nShadePalIndex;
                            }
                        }
                    }
                }
                else
                {
                    for (int y = 0; y < 8; y++)
                    {
                        for (int x = 0; x < 8; x++)
                        {
                            int posX = col * 8 + x;
                            int posY = row * 8 + y;
                            DWORD dwPos = posY * OUT_DC_WIDTH + posX;

                            if (m_pOutFontPixels[dwPos] == dwBgColor)
                            {
                                tilePixel.color[y][x] = m_nBgPalIndex;
                            }
                            else if (m_pOutFontPixels[dwPos] == COLOR_RGB_TO_BGR(g_PalColor[1]))
                            {
                                tilePixel.color[y][x] = 1;
                            }
                            else if (m_pOutFontPixels[dwPos] == COLOR_RGB_TO_BGR(g_PalColor[2]))
                            {
                                tilePixel.color[y][x] = 2;
                            }
                            else if (m_pOutFontPixels[dwPos] == COLOR_RGB_TO_BGR(g_PalColor[3]))
                            {
                                tilePixel.color[y][x] = 3;
                            }
                        }
                    }
                }

                //每一行颜色提取
                uint8_t colorData[16] = { 0 };
                for (int i = 0; i < 8; i++)
                {
                    BYTE color0 = tilePixel.color[i][0];
                    BYTE color1 = tilePixel.color[i][1];
                    BYTE color2 = tilePixel.color[i][2];
                    BYTE color3 = tilePixel.color[i][3];
                    BYTE color4 = tilePixel.color[i][4];
                    BYTE color5 = tilePixel.color[i][5];
                    BYTE color6 = tilePixel.color[i][6];
                    BYTE color7 = tilePixel.color[i][7];

                    BYTE colorL = 0;
                    BYTE colorH = 0;

                    colorL |= (color0 & 0x01) << 7;
                    colorH |= ((color0 & 0x02) << 6);

                    colorL |= (color1 & 0x01) << 6;
                    colorH |= ((color1 & 0x02) << 5);

                    colorL |= (color2 & 0x01) << 5;
                    colorH |= ((color2 & 0x02) << 4);

                    colorL |= (color3 & 0x01) << 4;
                    colorH |= ((color3 & 0x02) << 3);

                    colorL |= (color4 & 0x01) << 3;
                    colorH |= ((color4 & 0x02) << 2);

                    colorL |= (color5 & 0x01) << 2;
                    colorH |= ((color5 & 0x02) << 1);

                    colorL |= (color6 & 0x01) << 1;
                    colorH |= ((color6 & 0x02) << 0);

                    colorL |= (color7 & 0x01) << 0;
                    colorH |= ((color7 & 0x02) >> 1);

                    colorData[i] = colorL;
                    colorData[i + 8] = colorH;
                }

                // 单个 8*8 Tile 存入数据缓冲
                for (int i = 0; i < 16; i++)
                {
                    vChrData.push_back(colorData[i]);
                }

                if (vChrData.size() >= EXPORT_BUF_SIZE)
                {
                    fileHelper.Write(vChrData.data(), vChrData.size(), 0, 0, FILE_CURRENT);
                    vChrData.clear();
                }
            }
        }

        // 将最后多余行拷贝到前面
        RECT rtClear = { 0, 0, 128, 8 };
        ::FillRect(m_hBackDC, &rtClear, hBgBrush);
        ::BitBlt(m_hBackDC, 0, 0, 128, 8, m_hOutFontDC, 0, (nPosY / 8) * 8, SRCCOPY);
        ::BitBlt(m_hOutFontDC, 0, 0, 128, 8, m_hBackDC, 0, 0, SRCCOPY);
        nPosY = nPosY % 8;

        // 100毫秒更新一次进度, 过于频繁会影响性能
        curClock = clock();

        countCur += strItem.size();
        if (cbProcess && (curClock - lastClock >= 100 || (i + 1 == nTotalLine)))
        {
            double costTime = (double)(curClock - beginClock) / 1000.0f;
            static double lastSpeed = 0.0f;
            double remainTime = (((double)countTotal - (double)(countCur)) / lastSpeed);
            double progress = (double)((countCur) * 100) / (double)countTotal;

            // 速度统计
            if (curClock - lastSpeedClock >= 500)
            {
                lastSpeed = (double)(countCur - lastSpeedCount) / ((double)(curClock - lastSpeedClock) / 1000.0f);
                lastSpeedCount = countCur;
                lastSpeedClock = curClock;
            }
            else
            {
                lastSpeed = countCur / ((double)(curClock - lastSpeedClock) / 1000.0f);
            }

            if (isnan(remainTime))
            {
                remainTime = 0.0f;
            }

            if (!cbProcess(progress, countCur, countTotal, lastSpeed, costTime, remainTime) || m_fQuit)
            {
                break;
            }

            // 记录上次时间滴答
            lastClock = curClock;
        }
    }

    ::DeleteObject(hBgBrush);

    if (!vChrData.empty())
    {
        fileHelper.Write(vChrData.data(), vChrData.size(), 0, 0, FILE_CURRENT);
    }

    fileHelper.SetEnd(fileHelper.GetPointer());

    return true;
}

void CWindowDraw::_OutByMode(const _tstring& strItem, int nPosY)
{
    RECT rc = { 0, 0, 128, 128 };
    int nSrcX = 0;
    int nSrcY = 0;
    int nDestX = 0;
    int nDestY = 0;
    int nPosX = 0;
    int nPosDestX = 0;

    // 左右连续
    if (eChrOutMode::eOutHorizontal == m_eOutMode)
    {
        // 清空
        ::FillRect(m_hBufFontDC, &rc,  GetStockBrush(BLACK_BRUSH));

        // 单行文本处理
        for (int i = 0; i < strItem.size(); i++)
        {
            int nTileWidth = 16;
            int nTileHeight = 8;
            int nFontWidth = m_fontWidth;

            // 计算字符宽度
            if (strItem[i] < 0x80 && !m_fAsciiFull)
            {
                nFontWidth = m_fontWidth / 2;
                nTileWidth = nTileWidth / 2;
            }

            // 上部分 16/8 * 8
            {
                nSrcX = nPosX;
                nSrcY = nPosY;

                nDestX = nPosDestX;
                nDestY = 0;

                nDestX = nPosDestX % 128;
                nDestY = nPosDestX / 128 * nTileHeight;

                ::BitBlt(m_hBufFontDC, nDestX, nDestY, nFontWidth, nTileHeight, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                nPosDestX += nFontWidth;
            }

            // 下部分 16/8 * 8
            {
                nSrcX = nPosX;
                nSrcY = nPosY + nTileHeight;

                nDestX = nPosDestX;
                nDestY = 0;
                nDestX = nPosDestX % 128;
                nDestY = nPosDestX / 128 * nTileHeight;

                ::BitBlt(m_hBufFontDC, nDestX, nDestY, nFontWidth, nTileHeight, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                nPosDestX += nFontWidth;
            }

            nPosX += nFontWidth;
        }

        ::BitBlt(m_hOutFontDC, 0, nPosY, OUT_DC_WIDTH, m_fontHeight, m_hBufFontDC, 0, 0, SRCCOPY);
    }

    // 上下连续
    if (eChrOutMode::eOutVertical == m_eOutMode)
    {
        // 清空
        ::FillRect(m_hBufFontDC, &rc,  GetStockBrush(BLACK_BRUSH));

        // 单行文本处理
        for (int i = 0; i < strItem.size(); i++)
        {
            int nTileWidth = 16;
            int nTileHeight = 8;
            int nFontWidth = m_fontWidth;

            // 计算字符宽度
            if (strItem[i] < 0x80 && !m_fAsciiFull)
            {
                nFontWidth = m_fontWidth / 2;
                nTileWidth = nTileWidth / 2;
            }

            if (16 == nTileWidth)
            {
                // 左上部分 8 * 8
                {
                    nSrcX = nPosX;
                    nSrcY = nPosY;

                    nDestX = nPosDestX;
                    nDestY = 0;

                    nDestX = nPosDestX % 128;
                    nDestY = nPosDestX / 128 * 8;

                    ::BitBlt(m_hBufFontDC, nDestX, nDestY, 8, 8, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                    nPosDestX += 8;
                }

                // 左下部分 8 * 8
                {
                    nSrcX = nPosX;
                    nSrcY = nPosY + 8;

                    nDestX = nPosDestX;
                    nDestY = 0;

                    nDestX = nPosDestX % 128;
                    nDestY = nPosDestX / 128 * 8;

                    ::BitBlt(m_hBufFontDC, nDestX, nDestY, 8, 8, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                    nPosDestX += 8;
                }

                // 右上部分 8 * 8
                {
                    nSrcX = nPosX + 8;
                    nSrcY = nPosY;

                    nDestX = nPosDestX;
                    nDestY = 0;

                    nDestX = nPosDestX % 128;
                    nDestY = nPosDestX / 128 * 8;

                    ::BitBlt(m_hBufFontDC, nDestX, nDestY, 8, 8, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                    nPosDestX += 8;
                }

                // 右下部分 8 * 8
                {
                    nSrcX = nPosX + 8;
                    nSrcY = nPosY + 8;

                    nDestX = nPosDestX;
                    nDestY = 0;

                    nDestX = nPosDestX % 128;
                    nDestY = nPosDestX / 128 * 8;

                    ::BitBlt(m_hBufFontDC, nDestX, nDestY, 8, 8, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                    nPosDestX += 8;
                }
            }
            else
            {
                // 上部分 8 * 8
                {
                    nSrcX = nPosX;
                    nSrcY = nPosY;

                    nDestX = nPosDestX;
                    nDestY = 0;

                    nDestX = nPosDestX % 128;
                    nDestY = nPosDestX / 128 * nTileHeight;

                    ::BitBlt(m_hBufFontDC, nDestX, nDestY, 8, 8, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                    nPosDestX += 8;
                }

                // 下部分 8 * 8
                {
                    nSrcX = nPosX;
                    nSrcY = nPosY + nTileHeight;

                    nDestX = nPosDestX;
                    nDestY = 0;
                    nDestX = nPosDestX % 128;
                    nDestY = nPosDestX / 128 * nTileHeight;

                    ::BitBlt(m_hBufFontDC, nDestX, nDestY, 8, 8, m_hOutFontDC, nSrcX, nSrcY, SRCCOPY);
                    nPosDestX += 8;
                }
            }

            nPosX += nFontWidth;
        }

        ::BitBlt(m_hOutFontDC, 0, nPosY, OUT_DC_WIDTH, m_fontHeight, m_hBufFontDC, 0, 0, SRCCOPY);
    }

    // 上下压缩
    if (eChrOutMode::eOutVerticalCompress == m_eOutMode)
    {
        // 清空
        ::FillRect(m_hBufFontDC, &rc,  GetStockBrush(BLACK_BRUSH));

        for (int y = nPosY; y < nPosY + 8; y++)
        {
            for (int x = 0; x < OUT_DC_WIDTH; x++)
            {
                DWORD dwUp      = m_pOutFontPixels[y * OUT_DC_WIDTH + x];
                DWORD dwDown    = m_pOutFontPixels[ (y + 8) * OUT_DC_WIDTH + x];
                DWORD dwXor     = 0;

                if (dwUp && dwDown)
                {
                    dwXor = COLOR_RGB_TO_BGR(g_PalColor[3]);
                }
                else if (dwUp)
                {
                    dwXor = COLOR_RGB_TO_BGR(g_PalColor[1]);
                }
                else if (dwDown)
                {
                    dwXor = COLOR_RGB_TO_BGR(g_PalColor[2]);
                }

                m_pBufFontPixels[y * OUT_DC_WIDTH + x] = dwXor;
            }
        }

        ::BitBlt(m_hOutFontDC, 0, nPosY, OUT_DC_WIDTH, 16, m_hBufFontDC, 0, nPosY, SRCCOPY);
    }

    // 左右压缩
    if (eChrOutMode::eOutHorizontalCompress == m_eOutMode)
    {
        // 清空
        ::FillRect(m_hBufFontDC, &rc, GetStockBrush(BLACK_BRUSH));

        // 横向8个字符
        for (int i = 0; i < strItem.size(); i++)
        {
            for (int y = nPosY; y < nPosY + 16; y++)
            {
                for (int x = 0; x < 16; x++)
                {
                    DWORD dwLeft    = m_pOutFontPixels[y * OUT_DC_WIDTH + (i * 2 + 0) * 16 + x + 0];
                    DWORD dwRight   = m_pOutFontPixels[y * OUT_DC_WIDTH + (i * 2 + 0) * 16 + x + 16];
                    DWORD dwXor     = 0;

                    if (dwLeft && dwRight)
                    {
                        dwXor = COLOR_RGB_TO_BGR(g_PalColor[3]);
                    }
                    else if (dwLeft)
                    {
                        dwXor = COLOR_RGB_TO_BGR(g_PalColor[1]);
                    }
                    else if (dwRight)
                    {
                        dwXor = COLOR_RGB_TO_BGR(g_PalColor[2]);
                    }

                    m_pBufFontPixels[y * OUT_DC_WIDTH + i * 16 + x] = dwXor;
                }
            }
        }

        ::BitBlt(m_hOutFontDC, 0, nPosY, 128, 16, m_hBufFontDC, 0, nPosY, SRCCOPY);
    }
}