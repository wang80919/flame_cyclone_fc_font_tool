#pragma once
#include "CWindow/CDialogBase.h"
#include "resource.h"
#include <vector>
#include <windowsx.h>
#include <algorithm>
#include "CWindowDraw.h"
#include "Win32Utils/IniHelper.h"
#include "Win32Utils/CHttpsDownload.h"
#include "Common.h"

#define UPDATE_URL                           _T("https://gitee.com/flame_cyclone/fc_font_tool/raw/master/Release/update.json")

#define INPUT_TEXT_LENGTH_MAN               (1024 * 1024 * 16)

#define CONFIG_NAME_ROOT                    _T("config")

#define CONFIG_VIEW_GRID                    _T("view_grid")
#define CONFIG_VIEW_NO_RETURN               _T("view_no_return")
#define CONFIG_VIEW_NO_SPACE                _T("view_no_space")
#define CONFIG_VIEW_NO_DUPLICATE            _T("view_no_duplicate")
#define CONFIG_VIEW_ASCII_FULL              _T("view_ascii_full")
#define CONFIG_VIEW_START                   _T("view_start")

#define CONFIG_FONT_START                   _T("font_start")
#define CONFIG_FONT_NAME                    _T("font_name")
#define CONFIG_FONT_SIZE                    _T("font_size")
#define CONFIG_FONT_PIXEL                   _T("font_pixel")
#define CONFIG_FONT_WIDTH                   _T("font_width")
#define CONFIG_FONT_OUT_WIDTH               _T("font_out_width")
#define CONFIG_FONT_HEIGHT                  _T("font_height")
#define CONFIG_FONT_ORIENTATION             _T("font_Orientation")

#define CONFIG_OFFSET_X                     _T("offset_x")
#define CONFIG_OFFSET_Y                     _T("offset_y")

#define CONFIG_SHADE_X                      _T("shade_x")
#define CONFIG_SHADE_Y                      _T("shade_y")
#define CONFIG_SHADE_ENABLE                 _T("shade_enable")

#define CONFIG_FONT_BOLD                    _T("font_bold")
#define CONFIG_FONT_ITANLIC                 _T("font_itanlic")
#define CONFIG_FONT_UNDERLINE               _T("font_underline")
#define CONFIG_FONT_STRIKEOUT               _T("font_strikeout")

#define CONFIG_VIEW_SCROLL_POS              _T("view_scroll_pos")
#define CONFIG_TEXT_SCROLL_V_POS            _T("text_scroll_v_pos")
#define CONFIG_TEXT_SCROLL_H_POS            _T("text_scroll_h_pos")
#define CONFIG_TEXT_SEL_START_POS           _T("text_select_start_pos")
#define CONFIG_TEXT_SEL_END_POS             _T("text_select_end_pos")

#define CONFIG_WINDOW_X                     _T("window_x")
#define CONFIG_WINDOW_Y                     _T("window_y")
#define CONFIG_WINDOW_TOPMOST               _T("window_topmost")
#define CONFIG_WINDOW_DEBUG                 _T("window_debug")

#define CONFIG_CODE_COMMENT                 _T("code_comment")
#define CONFIG_CODE_X                       _T("code_x_2")
#define CONFIG_CODE_Y                       _T("code_y_2")
#define CONFIG_CODE_CENTER                  _T("code_center")
#define CONFIG_CODE_SIZE                    _T("code_size")
#define CONFIG_CODE_MASK                    _T("code_mask")

#define CONFIG_BG_COLOR                     _T("bg_color")
#define CONFIG_CH_COLOR                     _T("ch_color")
#define CONFIG_SHADE_COLOR                  _T("shade_color")

#define CONFIG_OUTPUT_MODE                  _T("output_mode")

#define CONFIG_OUTPUT_PATH                  _T("output_path")

#define HELP_ABOUT_URL                      _T("https://gitee.com/flame_cyclone/fc_font_tool")

typedef struct _UPDATE_INFO
{
    std::map<_tstring, _tstring> mapUpdateDesc;
    _tstring strVersion;
    _tstring strFileUrl;
    _tstring strSavePath;
    bool bForceUpdate = false;
}UPDATE_INFO;

class CMainFrame: public CDialogBase
{

public:

    LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);

    virtual LRESULT OnCommand(WORD wNotify, WORD wID, HWND hWnd);

    LRESULT OnFontStartChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnTextChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnInputTextChange(WORD wNotify, WORD wID, HWND hWnd);

    LRESULT OnNotifyFontSize(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyFontWidth(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyFontOutWidth(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyFontHeight(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyFontOrientation(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyOffsetXChange(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyOffsetYChange(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyShadeXChange(WORD wID, LPNMHDR pNmhdr);
    LRESULT OnNotifyShadeYChange(WORD wID, LPNMHDR pNmhdr);

    LRESULT OnFontSizeChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontWidthChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontOutWidthChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontHeightChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontOrientationChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontOffsetXChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontOffsetYChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontShadeXChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFontShadeYChange(WORD wNotify, WORD wID, HWND hWnd);

    LRESULT OnBgColorChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnChColorChange(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnShadeColorChange(WORD wNotify, WORD wID, HWND hWnd);

    LRESULT OnGrid(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnNoReturn(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnNoSpace(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnNoDuplicate(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnExportChr(WORD wNotify, WORD wID, HWND hWnd);

    LRESULT OnCheckFontStart(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontCodeComment(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontCodeX(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontCodeY(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontCodeCenter(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontCodeSize(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontCodeMask(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontAsciiFull(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontBold(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontItanlic(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontUnderline(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontStrikeOut(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnCheckFontShade(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnOutputMode(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnTopmost(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnDebug(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnQuit(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnAbout(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnUpdate(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFont_8_8(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFont_12_8(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFont_12_12(WORD wNotify, WORD wID, HWND hWnd);
    LRESULT OnFont_16_16(WORD wNotify, WORD wID, HWND hWnd);

    virtual BOOL PreTranslateMessage(LPMSG pMsg);

protected:

    DECLARE_DLG_MESSAGE_MAP()

private:

    _tstring _GetCfgDir();
    void _InitFontList();
    void _UpdateFont();
    void _LimitRange(WORD wEditID, WORD wSpinID, double& nVal, double nMin, double nMax);
    void _LimitRange(WORD wEditID, WORD wSpinID, int& nVal, int nMin, int nMax);
    bool _CuiProcess();
    _tstring _GetNameWithVersion(const _tstring strVer);
    _tstring _GetBackName();
    bool _UpdateCheck(bool fManual = false);

private:
    std::vector<LOGFONT> m_vLogFonts;                   // 字体信息列表
    CWindowDraw         m_wndDraw;                      // 绘制显示子窗口
    CHttpsDownload      m_dlTask;                       // 下载任务
    CIniHelper          m_cfg;                          // Ini配置
    HWND                m_hWndFontList;                 // 字体列表控件
    HWND                m_hWndEditText;                 // 文本编辑框
    UPDATE_INFO         m_UpdateInfo;

    bool                m_fBusy = false;
    bool                m_fQuit = false;
    bool                m_fCuiMode = false;
    bool                m_fCommandLaunch = false;
    bool                m_fCmdShow = false;
    bool                m_fUpdate = false;
    bool                m_fAutoChecking = false;
};

