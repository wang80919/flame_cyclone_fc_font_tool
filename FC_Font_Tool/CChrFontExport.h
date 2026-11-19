#pragma once

#include <wtypesbase.h>
#include <string>
#include <vector>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CChrFontExport
{
public:

    static bool SetFont(
        const _tstring strFont,     //字体名
        int nSize,                  //字号
        bool fWeight,               //字重
        bool fItanlic,              //斜体
        bool fUnderline,            //下划线
        bool fStrikeOut             //删除线
    );

    static bool ExportChr(
        const _tstring strContent,  //文本内容
        int nWidth,                 //字体宽度
        int nHeight                 //字体高度
    );

private:

    LOGFONT         m_lf;                       // 逻辑字体信息
    HFONT           m_hFont = NULL;             // 逻辑字体
};

