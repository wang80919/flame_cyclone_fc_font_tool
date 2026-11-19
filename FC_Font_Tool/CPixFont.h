#pragma once
#include <windows.h>
#include <tchar.h>
#include <string>

#pragma warning(disable:4100)

#ifdef UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CPixFont
{
public:

	CPixFont();
	~CPixFont();

	HDC GetDC() const;
	int GetWidth() const;
	int GetHeight() const;
	DWORD GetPixel(int x, int y) const;

	void SetSize(INT width, INT height);
	void SetFont(const LOGFONT* lplf, int size = 12);
	void SetFont(const _tstring name, int size = 12);
	SIZE PaintChar(WCHAR ch);

private:

	bool _CreateBitmap();

private:
	HDC			m_hdc = NULL;
	HBITMAP		m_hBitmap = NULL;
	HFONT		m_hFont = NULL;
	INT			m_nWidth = 0;
	INT			m_nHeight = 0;
	DWORD*		m_pPixels = NULL;
};

