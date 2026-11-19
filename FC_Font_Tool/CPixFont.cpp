#include "CPixFont.h"
#include <windowsx.h>
#include <strsafe.h>

CPixFont::CPixFont()
{
	m_hFont = NULL;
	HDC hdc = ::GetDC(NULL);
	m_hdc = ::CreateCompatibleDC(hdc);
	::ReleaseDC(NULL, hdc);
	::SetTextColor(m_hdc, RGB(255, 255, 255));
	::SetBkColor(m_hdc, RGB(0, 0, 0));
}

CPixFont::~CPixFont()
{
	if(m_hdc)
	{
		::DeleteDC(m_hdc);
	}

	if(m_hBitmap)
	{
		::DeleteObject(m_hBitmap);
	}

	if(m_hFont)
	{
		::DeleteObject(m_hFont);
	}
}

void CPixFont::SetSize(INT width, INT height)
{
	m_nWidth = width;
	m_nHeight= height;

	_CreateBitmap();
}

void CPixFont::SetFont(const LOGFONT* lplf, int size/* = 12*/)
{
	if (NULL != m_hFont)
	{
		::DeleteObject(m_hFont);
	}

	LOGFONT lf = *lplf;

	lf.lfWidth = 0;
	lf.lfHeight = ::MulDiv(size, ::GetDeviceCaps(m_hdc, LOGPIXELSY), 72);

	m_hFont = ::CreateFontIndirect(&lf);

	if (m_hFont)
	{
		::SelectObject(m_hdc, m_hFont);
	}
}

void CPixFont::SetFont(const _tstring name, int size/* = 12*/)
{
	LOGFONT lf = { 0 };
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfQuality = NONANTIALIASED_QUALITY;
	lf.lfHeight = ::MulDiv(size, ::GetDeviceCaps(m_hdc, LOGPIXELSY), 72);

	::StringCchCopy(lf.lfFaceName, _countof(lf.lfFaceName), name.c_str());

	m_hFont = ::CreateFontIndirect(&lf);

	if (m_hFont)
	{
		::SelectObject(m_hdc, m_hFont);
	}
}

HDC CPixFont::GetDC() const
{
	return m_hdc;
}

int CPixFont::GetWidth() const
{
	return m_nWidth;
}

int CPixFont::GetHeight() const
{
	return m_nHeight;
}

DWORD CPixFont::GetPixel(int x, int y) const
{
	if (x < m_nWidth && y < m_nHeight)
	{
		return m_pPixels[y * m_nWidth + x];
	}

	return 0;
}

SIZE CPixFont::PaintChar(WCHAR ch)
{
	RECT rc = { 0, 0, m_nWidth, m_nHeight };
	SIZE sz = { 0 };

	::SetRect(&rc, 0, 0, m_nWidth, m_nHeight);
	::FillRect(m_hdc, &rc,(HBRUSH)GetStockObject(BLACK_BRUSH));

	::TextOut(m_hdc, 0, 0, &ch, 1);

	::DrawText(m_hdc, &ch, 1, &rc, DT_CALCRECT);
	::GetTextExtentPoint32(m_hdc, &ch, 1, &sz);
		
	sz.cx = rc.right;
	sz.cy = rc.bottom;
	return sz;
}

bool CPixFont::_CreateBitmap()
{
	HBITMAP hBitmap;
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_nWidth;
	bmi.bmiHeader.biHeight = -m_nHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	hBitmap = ::CreateDIBSection(m_hdc, &bmi, DIB_RGB_COLORS, (void**)&m_pPixels, NULL, 0);

	if (NULL == hBitmap)
	{
		return false;
	}

	if(m_hBitmap != NULL)
	{
		::DeleteObject(m_hBitmap);
	}

	::SelectObject(m_hdc, hBitmap);
	m_hBitmap = hBitmap;

	return true;
}