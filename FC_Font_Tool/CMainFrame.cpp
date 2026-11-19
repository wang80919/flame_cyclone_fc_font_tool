#include "include/rapidjson/document.h"
#include "include/rapidjson/prettywriter.h"
#include "include/rapidjson/filereadstream.h"
#include "include/rapidjson/istreamwrapper.h"

#include "CMainFrame.h"
#include "Win32Utils/CComFileDlg.h"
#include "Win32Utils/CPathUtils.h"
#include "Win32Utils/CStrUtils.h"
#include "Win32Utils/CProcessUtils.h"
#include "Win32Utils/CBytesUtils.h"
#include "Win32Utils/CCommandLine.h"
#include "Win32Utils/CFileVersion.h"
#include "Win32Utils/MessageBoxCenter.h"

BEGIN_DLG_MESSAGE_MAP(CMainFrame, CDialogBase)
    ON_DLG_MESSAGE(WM_INITDIALOG, &CMainFrame::OnInitDialog)
    ON_DLG_MESSAGE(WM_CLOSE, &CMainFrame::OnClose)
    ON_DLG_MESSAGE(WM_DRAWITEM, &CMainFrame::OnDrawItem)

    ON_DLG_COMMAND_RANGE(IDC_BUTTON_BG_0, IDC_BUTTON_BG_3, &CMainFrame::OnBgColorChange)
    ON_DLG_COMMAND_RANGE(IDC_BUTTON_CH_0, IDC_BUTTON_CH_3, &CMainFrame::OnChColorChange)
    ON_DLG_COMMAND_RANGE(IDC_BUTTON_SHADE_0, IDC_BUTTON_SHADE_3, &CMainFrame::OnShadeColorChange)
    ON_DLG_COMMAND_RANGE(IDC_RADIO_OUT_NORMAL, IDC_RADIO_OUT_VC, &CMainFrame::OnOutputMode)

    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_START, &CMainFrame::OnFontStartChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_TEXT, &CMainFrame::OnTextChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_CODE_IN, &CMainFrame::OnInputTextChange)
    ON_DLG_COMMAND_NOTIFY(CBN_SELCHANGE, IDC_COMBO_FONT_LIST, &CMainFrame::OnFontChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_FONT_SIZE, &CMainFrame::OnFontSizeChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_FONT_WIDTH, &CMainFrame::OnFontWidthChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_FONT_OUT_WIDTH, &CMainFrame::OnFontOutWidthChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_FONT_HEIGHT, &CMainFrame::OnFontHeightChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_FONT_ORIENTATION, &CMainFrame::OnFontOrientationChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_OFFSET_X, &CMainFrame::OnFontOffsetXChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_OFFSET_Y, &CMainFrame::OnFontOffsetYChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_SHADE_X, &CMainFrame::OnFontShadeXChange)
    ON_DLG_COMMAND_NOTIFY(EN_CHANGE, IDC_EDIT_SHADE_Y, &CMainFrame::OnFontShadeYChange)

    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FONT_SIZE, &CMainFrame::OnNotifyFontSize)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FONT_WIDTH, &CMainFrame::OnNotifyFontWidth)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FONT_OUT_WIDTH, &CMainFrame::OnNotifyFontOutWidth)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FONT_HEIGHT, &CMainFrame::OnNotifyFontHeight)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FONT_ORIENTATION, &CMainFrame::OnNotifyFontOrientation)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_OFFSET_X, &CMainFrame::OnNotifyOffsetXChange)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_OFFSET_Y, &CMainFrame::OnNotifyOffsetYChange)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHADE_X, &CMainFrame::OnNotifyShadeXChange)
    ON_DLG_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHADE_Y, &CMainFrame::OnNotifyShadeYChange)

    ON_DLG_COMMAND(IDC_CHECK_START, &CMainFrame::OnCheckFontStart)
    ON_DLG_COMMAND(IDC_CHECK_ASCII_FULL_WIDTH, &CMainFrame::OnCheckFontAsciiFull)
    ON_DLG_COMMAND(IDC_CHECK_BOLD, &CMainFrame::OnCheckFontBold)
    ON_DLG_COMMAND(IDC_CHECK_ITANLIC, &CMainFrame::OnCheckFontItanlic)
    ON_DLG_COMMAND(IDC_CHECK_UNDERLINE, &CMainFrame::OnCheckFontUnderline)
    ON_DLG_COMMAND(IDC_CHECK_STRIKEOUT, &CMainFrame::OnCheckFontStrikeOut)
    ON_DLG_COMMAND(IDC_CHECK_SHADE, &CMainFrame::OnCheckFontShade)
    ON_DLG_COMMAND(IDC_CHECK_GRID, &CMainFrame::OnGrid)
    ON_DLG_COMMAND(IDC_CHECK_NORETURN, &CMainFrame::OnNoReturn)
    ON_DLG_COMMAND(IDC_CHECK_NOSPACE, &CMainFrame::OnNoSpace)
    ON_DLG_COMMAND(IDC_CHECK_NO_DUPLICATE, &CMainFrame::OnNoDuplicate)
    ON_DLG_COMMAND(IDC_BUTTON_EXPORT_CHR, &CMainFrame::OnExportChr)
    ON_DLG_COMMAND(IDC_CHECK_COMMENT, &CMainFrame::OnCheckFontCodeComment)
    ON_DLG_COMMAND(IDC_CHECK_CODE_X, &CMainFrame::OnCheckFontCodeX)
    ON_DLG_COMMAND(IDC_CHECK_CODE_Y, &CMainFrame::OnCheckFontCodeY)
    ON_DLG_COMMAND(IDC_CHECK_CODE_CENTER, &CMainFrame::OnCheckFontCodeCenter)
    ON_DLG_COMMAND(IDC_CHECK_CODE_SIZE, &CMainFrame::OnCheckFontCodeSize)
    ON_DLG_COMMAND(IDC_CHECK_CODE_MASK, &CMainFrame::OnCheckFontCodeMask)

    ON_DLG_COMMAND(ID_SETTING_TOPMOST, &CMainFrame::OnTopmost)
    ON_DLG_COMMAND(ID_SETTING_DEBUG, &CMainFrame::OnDebug)
    ON_DLG_COMMAND(ID_FILE_EXIT, &CMainFrame::OnQuit)
    ON_DLG_COMMAND(ID_HELP_ABOUT, &CMainFrame::OnAbout)
    ON_DLG_COMMAND(ID_HELP_UPDATE, &CMainFrame::OnUpdate)

    ON_DLG_COMMAND(IDC_BUTTON_8_8, &CMainFrame::OnFont_8_8)
    ON_DLG_COMMAND(IDC_BUTTON_12_8, &CMainFrame::OnFont_12_8)
    ON_DLG_COMMAND(IDC_BUTTON_12_12, &CMainFrame::OnFont_12_12)
    ON_DLG_COMMAND(IDC_BUTTON_16_16, &CMainFrame::OnFont_16_16)

END_DLG_MESSAGE_MAP()

_tstring CMainFrame::_GetCfgDir()
{
    _tstring strDir = CPathUtils::GetCurrentModuleDir() + _T("\\") + CPathUtils::GetCurrentModuleName();

    if (!CPathUtils::IsDirectory(strDir))
    {
        CPathUtils::CreateDir(strDir);
    }

    return strDir;
}

void CMainFrame::_InitFontList()
{
    HDC hdc = ::GetDC(m_hWnd);

    LOGFONT logFont = { 0 };
    logFont.lfFaceName[0] = '\0';
    logFont.lfCharSet = DEFAULT_CHARSET;

    std::vector<LOGFONT> vLogFonts;                   // 字体信息列表

    EnumFontFamiliesEx(hdc, &logFont, [](CONST LOGFONT* lpelfe, CONST TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam) -> int {
        std::vector<LOGFONT>* plogFonts = (std::vector<LOGFONT>*)lParam;
        plogFonts->push_back(*lpelfe);
        return TRUE;

        }, (LPARAM)&vLogFonts, 0);

    std::sort(vLogFonts.begin(), vLogFonts.end(), [](const LOGFONT& r1, const LOGFONT& r2) {
        return _tcsicmp(r1.lfFaceName, r2.lfFaceName) < 0;
        }
    );

    CIniHelper cfg;
    _tstring strFontName = m_cfg.GetString(CONFIG_NAME_ROOT, CONFIG_FONT_NAME, _T("宋体"));
    if (strFontName.empty() && !vLogFonts.empty())
    {
        strFontName = _T("宋体");
    }

    int nIndex = 0;
    bool fSelect = false;
    std::set<_tstring> setFontName;
    for (auto& item : vLogFonts)
    {
        auto it = setFontName.find(item.lfFaceName);
        if (it == setFontName.end())
        {
            m_vLogFonts.push_back(item);
            setFontName.insert(item.lfFaceName);
        }
    }

    for (auto& item : m_vLogFonts)
    {
        ComboBox_InsertString(m_hWndFontList, nIndex, item.lfFaceName);
        if (0 == _tcsicmp(item.lfFaceName, strFontName.c_str()))
        {
            ComboBox_SetCurSel(m_hWndFontList, nIndex);
            fSelect = true;
        }

        nIndex++;
    }

    if (!fSelect && !m_vLogFonts.empty())
    {
        ComboBox_SetCurSel(m_hWndFontList, m_vLogFonts.size() - 1);
    }

    ::ReleaseDC(m_hWnd, hdc);
}

LRESULT CMainFrame::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    m_cfg.SetPath(_GetCfgDir() + _T("\\") + CPathUtils::GetCurrentModuleName() + _T(".ini"), true);

    LoadAccel(IDR_ACCELERATOR);

    FILE_VERSION_INFO curVersionInfo = CFileVersion::GetCurrentModuleVersionInfo();
    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetWindowText(m_hWnd, szBuf, _countof(szBuf));
    _tstring strCaption = szBuf;
    strCaption += _T(" ");
    strCaption += curVersionInfo.strProductVersion;
    ::SetWindowText(m_hWnd, strCaption.c_str());

    //::SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);
    RECT rect = { 0, 0, 512, 512 };
    m_wndDraw.Register(_T("WindowDraw"));
    DWORD dwWndStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL;
    ::AdjustWindowRect(&rect, dwWndStyle, NULL);
    WORD cxClient = rect.right - rect.left + ::GetSystemMetrics(SM_CXVSCROLL);
    WORD cyClient = rect.bottom - rect.top;
    m_wndDraw.Create(m_hWnd, NULL, _T(""), SW_SHOW, dwWndStyle, 0, 16, 96, cxClient, cyClient, false);

    m_wndDraw.SetOnTextChange([this](int nCount, int nOutSize) {
        ;
        _tstring strContent = CStrUtils::Format(_T("输出预览 字模数量: %d 输出大小: %s"), nCount, CStrUtils::FormatByteSize(nOutSize).strUnitStr.c_str());
        SetItemString(IDC_STATIC_GROUP_VIEW, strContent);
        });

    m_wndDraw.SetOnPaintFinish([this]() {
        PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
        }
    );

    m_hWndFontList = GetItem(IDC_COMBO_FONT_LIST);
    m_hWndEditText = GetItem(IDC_EDIT_TEXT);
    ::SendMessage(m_hWndEditText, EM_LIMITTEXT, INPUT_TEXT_LENGTH_MAN, 0);

    // 初始化字体列表
    _InitFontList();

    if (_CuiProcess())
    {
        m_bShow = false;
        PostMessage(WM_CLOSE, 0, 0);
        return FALSE;
    }

    CHANGEFILTERSTRUCT cfs = { 0 };
    cfs.cbSize = sizeof(cfs);

    //允许低权限进程拖动文件进来
    ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, &cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, &cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA - 1, MSGFLT_ALLOW, &cfs);

    // 设置图标
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    if (hIcon)
    {
        ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    m_wndDraw.m_fontOutWidth = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_OUT_WIDTH, 128);
    m_wndDraw.m_fontSize = m_cfg.GetFloat(CONFIG_NAME_ROOT, CONFIG_FONT_SIZE, 11);
    m_wndDraw.m_fontWidth = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_WIDTH, 16);
    m_wndDraw.m_fontHeight = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_HEIGHT, 16);
    m_wndDraw.m_fontOrientation = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_ORIENTATION, 0);
    m_wndDraw.m_offsetX = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_OFFSET_X, 0);
    m_wndDraw.m_offsetY = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_OFFSET_Y, 0);
    m_wndDraw.m_shadeX = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_X, 0);
    m_wndDraw.m_shadeY = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_Y, 0);
    m_wndDraw.m_eOutMode = (eChrOutMode)m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_OUTPUT_MODE, 0);

    m_wndDraw.m_fGrid = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_GRID, 1);
    if (m_wndDraw.m_fGrid)
    {
        CheckButton(IDC_CHECK_GRID, true);
    }

    CheckButton(m_wndDraw.m_eOutMode + IDC_RADIO_OUT_NORMAL, true);

    m_wndDraw.m_fShade = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_ENABLE, 0);
    if (m_wndDraw.m_fShade)
    {
        CheckButton(IDC_CHECK_SHADE, true);
    }

    m_wndDraw.m_fNoReturn = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_NO_RETURN, 0);
    if (m_wndDraw.m_fNoReturn)
    {
        CheckButton(IDC_CHECK_NORETURN, true);
    }

    m_wndDraw.m_fNoSpace = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_NO_SPACE, 0);
    if (m_wndDraw.m_fNoSpace)
    {
        CheckButton(IDC_CHECK_NOSPACE, true);
    }

    m_wndDraw.m_fNoDuplicate = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_NO_DUPLICATE, 0);
    if (m_wndDraw.m_fNoDuplicate)
    {
        CheckButton(IDC_CHECK_NO_DUPLICATE, true);
    }

    m_wndDraw.m_fStartChar = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_START, 0);
    if (m_wndDraw.m_fStartChar)
    {
        CheckButton(IDC_CHECK_START, true);
    }

    m_wndDraw.m_fAsciiFull = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_ASCII_FULL, 0);
    if (m_wndDraw.m_fAsciiFull)
    {
        CheckButton(IDC_CHECK_ASCII_FULL_WIDTH, true);
    }

    m_wndDraw.m_fontBold = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_BOLD, 0);
    if (m_wndDraw.m_fontBold)
    {
        CheckButton(IDC_CHECK_BOLD, true);
    }

    m_wndDraw.m_fontItanlic = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_ITANLIC, 0);
    if (m_wndDraw.m_fontItanlic)
    {
        CheckButton(IDC_CHECK_ITANLIC, true);
    }

    m_wndDraw.m_fontUnderLine = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_UNDERLINE, 0);
    if (m_wndDraw.m_fontUnderLine)
    {
        CheckButton(IDC_CHECK_UNDERLINE, true);
    }

    m_wndDraw.m_fontStrikeOut = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_STRIKEOUT, 0);
    if (m_wndDraw.m_fontStrikeOut)
    {
        CheckButton(IDC_CHECK_STRIKEOUT, true);
    }

    m_wndDraw.m_fontStart = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_START, 0);

    m_wndDraw.m_fCodeComment = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CODE_COMMENT, -1);
    m_wndDraw.m_fCodeX2 = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CODE_X, -1);
    m_wndDraw.m_fCodeY2 = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CODE_Y, -1);
    m_wndDraw.m_fCodeCenter = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CODE_CENTER, -1);
    m_wndDraw.m_fCodeSize = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CODE_SIZE, -1);
    m_wndDraw.m_fCodeMask = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CODE_MASK, -1);
    CheckButton(IDC_CHECK_COMMENT, m_wndDraw.m_fCodeComment);
    CheckButton(IDC_CHECK_CODE_X, m_wndDraw.m_fCodeX2);
    CheckButton(IDC_CHECK_CODE_Y, m_wndDraw.m_fCodeY2);
    CheckButton(IDC_CHECK_CODE_CENTER, m_wndDraw.m_fCodeCenter);
    CheckButton(IDC_CHECK_CODE_SIZE, m_wndDraw.m_fCodeSize);
    CheckButton(IDC_CHECK_CODE_MASK, m_wndDraw.m_fCodeMask);

    SetItemInt(IDC_EDIT_OFFSET_X, m_wndDraw.m_offsetX);
    SetItemInt(IDC_EDIT_OFFSET_Y, m_wndDraw.m_offsetY);
    SetItemFloat(IDC_EDIT_FONT_SIZE, m_wndDraw.m_fontSize);
    SetItemInt(IDC_EDIT_FONT_WIDTH, m_wndDraw.m_fontWidth);
    SetItemInt(IDC_EDIT_FONT_HEIGHT, m_wndDraw.m_fontHeight);
    SetItemInt(IDC_EDIT_FONT_ORIENTATION, m_wndDraw.m_fontOrientation);
    SetItemInt(IDC_EDIT_SHADE_X, m_wndDraw.m_shadeX);
    SetItemInt(IDC_EDIT_SHADE_Y, m_wndDraw.m_shadeY);
    SetItemInt(IDC_EDIT_FONT_OUT_WIDTH, m_wndDraw.m_fontOutWidth);

    SendItemMessage(IDC_SPIN_OFFSET_X, UDM_SETRANGE32, -32, 32);
    SendItemMessage(IDC_SPIN_OFFSET_Y, UDM_SETRANGE32, -32, 32);
    SendItemMessage(IDC_SPIN_SHADE_X, UDM_SETRANGE32, -32, 32);
    SendItemMessage(IDC_SPIN_SHADE_Y, UDM_SETRANGE32, -32, 32);
    SendItemMessage(IDC_SPIN_FONT_SIZE, UDM_SETRANGE32, 1, 128);
    SendItemMessage(IDC_SPIN_FONT_WIDTH, UDM_SETRANGE32, 1, 128);
    SendItemMessage(IDC_SPIN_FONT_HEIGHT, UDM_SETRANGE32, 1, 128);
    SendItemMessage(IDC_SPIN_FONT_ORIENTATION, UDM_SETRANGE32, -180, 180);
    SendItemMessage(IDC_SPIN_FONT_OUT_WIDTH, UDM_SETRANGE32, 1, 256);

    SendItemMessage(IDC_EDIT_FONT_SIZE, UDM_SETPOS32, 0, m_wndDraw.m_fontSize);
    SendItemMessage(IDC_SPIN_FONT_WIDTH, UDM_SETPOS32, 0, m_wndDraw.m_fontWidth);
    SendItemMessage(IDC_SPIN_FONT_HEIGHT, UDM_SETPOS32, 0, m_wndDraw.m_fontHeight);
    SendItemMessage(IDC_SPIN_FONT_ORIENTATION, UDM_SETPOS32, 0, m_wndDraw.m_fontOrientation);
    SendItemMessage(IDC_SPIN_OFFSET_X, UDM_SETPOS32, 0, m_wndDraw.m_offsetX);
    SendItemMessage(IDC_SPIN_OFFSET_Y, UDM_SETPOS32, 0, m_wndDraw.m_offsetY);
    SendItemMessage(IDC_SPIN_SHADE_X, UDM_SETPOS32, 0, m_wndDraw.m_shadeX);
    SendItemMessage(IDC_SPIN_SHADE_Y, UDM_SETPOS32, 0, m_wndDraw.m_shadeY);
    SendItemMessage(IDC_SPIN_FONT_OUT_WIDTH, UDM_SETPOS32, 0, m_wndDraw.m_fontOutWidth);

    SetItemInt(IDC_EDIT_START, m_wndDraw.m_fontStart);

    _UpdateFont();

    std::string strPathPreFixed = CStrUtils::TStrToAStr(_GetCfgDir() + _T("\\") + CPathUtils::GetCurrentModuleName());
    _tstring strContent = CStrUtils::U8StrToTStr(CStrUtils::FromFileA(strPathPreFixed + ".txt"));
    if (strContent.empty())
    {
        strContent = _T(R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)");
    }

    SetItemString(IDC_EDIT_TEXT, strContent.c_str());
    SetItemString(IDC_EDIT_CODE_IN, CStrUtils::U8StrToTStr(CStrUtils::FromFileA(strPathPreFixed + "_Input.txt")).c_str());

    OnTextChange(0, 0, 0);

    m_wndDraw.SetText(strContent);
    m_wndDraw.SetScrollPos(m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_SCROLL_POS, 0));

    DWORD dwStartPos = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SEL_START_POS, 0);
    DWORD dwEndPos = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SEL_END_POS, 0);
    ::SendMessage(m_hWndEditText, EM_SETSEL, (WPARAM)dwStartPos, (LPARAM)dwEndPos);

    int vScroll = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SCROLL_V_POS, 0);
    int hScroll = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SCROLL_H_POS, 0);

    ::PostMessage(m_hWndEditText, EM_LINESCROLL, hScroll, vScroll);

    RECT rtDesk = { 0 };
    ::GetWindowRect(::GetDesktopWindow(), &rtDesk);
    RECT rtFrame = { 0 };
    ::GetWindowRect(m_hWnd, &rtFrame);

    LONG posX = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_X, -1);
    LONG posY = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_Y, -1);

    if (!(- 1 == posX && -1 == posY))
    {
        if (posX < 0) posX = 0;
        if (posY < 0) posY = 0;

        if (posX > rtDesk.right - (rtFrame.right - rtFrame.left)) posX = rtDesk.right - (rtFrame.right - rtFrame.left);
        if (posY > rtDesk.bottom - (rtFrame.bottom - rtFrame.top)) posY = rtDesk.bottom - (rtFrame.bottom - rtFrame.top);

        ::MoveWindow(m_hWnd, posX, posY, rtFrame.right - rtFrame.left, rtFrame.bottom - rtFrame.top, true);
    }
    else
    {
        MoveToCenter(nullptr);
    }

    m_wndDraw.m_nBgPalIndex = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_BG_COLOR, 0);
    m_wndDraw.m_nChPalIndex = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_CH_COLOR, 1);
    m_wndDraw.m_nShadePalIndex = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_COLOR, 2);

    if (m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_TOPMOST, 0))
    {
        PostCommand(ID_SETTING_TOPMOST, 0, 0);
    }

    if (m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_DEBUG, 0))
    {
        PostCommand(ID_SETTING_DEBUG, 0, 0);
    }

    m_wndDraw.UpdateSync();

    std::thread([this]()
        {
            while (!m_fQuit)
            {
                if (!_UpdateCheck())
                {
                    break;
                }

                ::Sleep(5000);
            }

        }).detach();

    return FALSE;
}

bool CMainFrame::_CuiProcess()
{
    std::vector<_tstring> strCmd = CCommandLine::GetCommandList();
    m_fCuiMode = CProcessUtils::IsWindowsCUI(CProcessUtils::GetParentID(GetCurrentProcessId()));
    if (!m_fCuiMode)
    {
        return false;
    }

    FILE* fp = NULL;
    if (!::AttachConsole(ATTACH_PARENT_PROCESS))
    {
        return true;
    }

    ::freopen_s(&fp, "CONOUT$", "wt", stdout); // 将stdout重定向到控制台窗口

    ::_tprintf(_T("FC字模助手(By FlameCyclone)\r\n"));
    ::_tprintf(_T("usage example: FC_Font_Tool.exe -input list.txt -output lit.chr -ini config.ini\r\n"));
    ::_tprintf(_T("-input   :Specify the text file\r\n"));
    ::_tprintf(_T("-output  :Specify the output .CHR file\r\n"));
    ::_tprintf(_T("-ini     :Specify the profile\r\n"));
    ::_tprintf(_T("-show    :Show window\r\n"));
    ::_tprintf(_T("\r\n"));

    if (!strCmd.empty())
    {
        m_fCommandLaunch = true;
        _tstring cfgPath;
        _tstring outFile = _T("out.chr");
        _tstring inFile;

        for (int i = 0; i < strCmd.size(); i++)
        {
            if (0 == _tcsicmp(strCmd[i].c_str(), _T("-ini")))
            {
                if (i + 1 < strCmd.size())
                {
                    cfgPath = strCmd[++i];
                    continue;
                }
            }

            if (0 == _tcsicmp(strCmd[i].c_str(), _T("-input")) ||
                0 == _tcsicmp(strCmd[i].c_str(), _T("-in")))
            {
                if (i + 1 < strCmd.size())
                {
                    inFile = strCmd[++i];
                    continue;
                }
            }

            if (0 == _tcsicmp(strCmd[i].c_str(), _T("-output")) ||
                0 == _tcsicmp(strCmd[i].c_str(), _T("-out")))
            {
                if (i + 1 < strCmd.size())
                {
                    outFile = strCmd[++i];
                    continue;
                }
            }

            if (0 == _tcsicmp(strCmd[i].c_str(), _T("-show")))
            {
                m_fCmdShow = true;
                return false;
                continue;
            }
        }

        _tstring strOutDir = CPathUtils::GetFileDir(outFile);
        if (!CPathUtils::IsDirectory(strOutDir))
        {
            CPathUtils::CreateDir(strOutDir);
        }

        CIniHelper cfg;
        cfg.SetPath(cfgPath, false);

        m_wndDraw.m_fontOutWidth = m_cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_OUT_WIDTH, 128);
        m_wndDraw.m_fontSize = cfg.GetFloat(CONFIG_NAME_ROOT, CONFIG_FONT_SIZE, 11);
        m_wndDraw.m_fontWidth = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_WIDTH, 16);
        m_wndDraw.m_fontHeight = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_HEIGHT, 16);
        m_wndDraw.m_fontOrientation = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_FONT_ORIENTATION, 0);
        m_wndDraw.m_offsetX = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_OFFSET_X, 0);
        m_wndDraw.m_offsetY = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_OFFSET_Y, 0);
        m_wndDraw.m_shadeX = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_X, 0);
        m_wndDraw.m_shadeY = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_Y, 0);
        m_wndDraw.m_fGrid = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_GRID, 1);
        m_wndDraw.m_fShade = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_ENABLE, 0);
        m_wndDraw.m_fAsciiFull = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_ASCII_FULL, 0);
        m_wndDraw.m_eOutMode = (eChrOutMode)cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_OUTPUT_MODE, 0);
        m_wndDraw.m_fStartChar = cfg.GetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_START, 0);

        _tstring strFontName = cfg.GetString(CONFIG_NAME_ROOT, CONFIG_OUTPUT_MODE, _T("宋体"));
        if (strFontName.empty() && !m_vLogFonts.empty())
        {
            strFontName = _T("宋体");
        }

        LOGFONT lf = { 0 };
        for (auto& item : m_vLogFonts)
        {
            if (0 == _tcsicmp(item.lfFaceName, strFontName.c_str()))
            {
                lf = item;
            }
        }

        lf.lfWidth = 0;
        lf.lfQuality = NONANTIALIASED_QUALITY;
        lf.lfItalic = m_wndDraw.m_fontItanlic;
        lf.lfWeight = m_wndDraw.m_fontBold ? FW_BOLD : 0;
        lf.lfUnderline = m_wndDraw.m_fontUnderLine;
        lf.lfStrikeOut = m_wndDraw.m_fontStrikeOut;
        m_wndDraw.SetFont(&lf);

        _tstring strContent;
        if (CPathUtils::IsArchive(inFile))
        {
            std::vector<uint8_t> vData;
            if (!CBytesUtils::BytesFromFile(vData, inFile))
            {
                return true;
            }

            int nNullCount = 0;
            for (const auto& item : vData)
            {
                if (0 == item)
                {
                    nNullCount++;
                }
            }

            vData.push_back('\0');
            vData.push_back('\0');

            if (nNullCount > 1)
            {
                strContent = (LPCTSTR)vData.data();
                m_wndDraw.SetText(strContent);
            }
            else
            {
                std::string strText = (const char*)vData.data();

                strContent;
                if (CStrUtils::GetUtf8Count(strText.c_str()))
                {
                    strContent = CStrUtils::U8StrToTStr(strText);
                }
                else
                {
                    strContent = CStrUtils::AStrToTStr(strText);
                }

                m_wndDraw.SetText(strContent);
            }

            clock_t timeBegin = ::clock();
            int nTotalCount = 0;
            m_wndDraw.ExportChr(outFile, [this, &nTotalCount](double progress,
                size_t cur,
                size_t total,
                double speed,
                double costTime,
                double remainingTime) {

                    nTotalCount = total;
                    static _tstring strProgress;

                    strProgress = CStrUtils::Format(_T("进度: %.1lf%% %zu/%zu 速度: %.0lf/s 耗时: %.1lfs 剩余: %.1lfs\r\n"),
                        (double)(cur * 100) / (double)total,
                        cur, total,
                        speed,
                        costTime,
                        remainingTime
                    );

                    ::_tprintf(_T("%s"), strProgress.c_str());
                    return true;

                }
            );
            clock_t timeEnd = ::clock();
            _tstring strMessage = CStrUtils::Format(_T("保存成功, 文件: %s, 耗时: %.3f秒, 共计 %zu 个字模, 平均速度: %.3lf/s\r\n"),
                outFile.c_str(),
                (double)(timeEnd - timeBegin) / 1000.0f, nTotalCount, (double)nTotalCount / ((double)(timeEnd - timeBegin) / 1000.0f)
            );

            ::_tprintf(_T("%s"), strMessage.c_str());
        }
    }

    ::FreeConsole();

    return true;
}

_tstring CMainFrame::_GetNameWithVersion(const _tstring strVer)
{
    _tstring strPath;
    _tstring strName = CPathUtils::GetCurrentModuleName(false);
    strPath = CPathUtils::GetCurrentModuleDir();
    strPath += _T("\\");
    strPath += strName;
    strPath += _T("_");
    strPath += strVer;
    strPath += _T("_");
#ifdef _WIN64
    strPath += _T("x64");
#else
    strPath += _T("x86");
#endif
    strPath += _T(".exe");

    return strPath;
}

_tstring CMainFrame::_GetBackName()
{
    _tstring strTemp = CPathUtils::GetCurrentModuleDir()
        + _T("\\")
        + CPathUtils::GetCurrentModuleName(false)
        + _T("_bak.exe")
        ;

    return strTemp;
}

bool CMainFrame::_UpdateCheck(bool fManual/* = false*/)
{
    FILE_VERSION_INFO curVersionInfo = CFileVersion::GetCurrentModuleVersionInfo();

    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetWindowText(m_hWnd, szBuf, _countof(szBuf));
    _tstring strCaption = szBuf;

    _tstring strTemp = _GetBackName();

    CPathUtils::Delete(strTemp);

    if (m_fCuiMode && !m_fCmdShow)
    {
        return false;
    }

    std::string strBuf;
    strBuf.resize(1048576);

    if (m_dlTask.DownloadToBuffer(UPDATE_URL, &strBuf[0], strBuf.size()))
    {
        rapidjson::Document document;
        UPDATE_INFO info;

        try
        {
            document.Parse(strBuf.c_str());

            if (document.HasMember("version") && document["version"].IsString())
            {
                info.strVersion = CStrUtils::U8StrToTStr(document["version"].GetString());
            }


#ifdef _WIN64
            if (document.HasMember("url") && document["url"].IsString())
            {
                info.strFileUrl = CStrUtils::U8StrToTStr(document["url"].GetString());
            }
#else
            if (document.HasMember("url_x86") && document["url_x86"].IsString())
            {
                info.strFileUrl = CStrUtils::U8StrToTStr(document["url_x86"].GetString());
            }
#endif

            if (document.HasMember("update_describe") && document["update_describe"].IsObject())
            {
                for (auto& item : document["update_describe"].GetObj())
                {
                    info.mapUpdateDesc.insert(
                        std::make_pair(CStrUtils::U8StrToTStr(item.name.GetString()),
                            CStrUtils::U8StrToTStr(item.value.GetString())
                        ));
                }
            }

            m_UpdateInfo = info;
            m_UpdateInfo.strSavePath = _GetNameWithVersion(m_UpdateInfo.strVersion);

            FILE_VERSION_INFO curVersionInfo = CFileVersion::GetCurrentModuleVersionInfo();
            CVersionNumber onlineVersionNumber(m_UpdateInfo.strVersion);
            CVersionNumber localVersionNumber(curVersionInfo.FileVerNumber);
            
            _tstring strUpdate;
            if (!m_UpdateInfo.mapUpdateDesc.empty())
            {
                strUpdate = m_UpdateInfo.mapUpdateDesc[_T("zh-cn")];
            }

            _tstring strCaption = CStrUtils::Format(_T("当前版本: %s, 是否更新到: %s 版本?"), 
                localVersionNumber.GetString().c_str(),
                m_UpdateInfo.strVersion.c_str());

            _tstring strInfo = CStrUtils::Format(_T("更新内容: \r\n%s"), strUpdate.c_str());

            CMessageBoxCenter boxCenter;
            if (onlineVersionNumber > localVersionNumber)
            {
                if (!fManual)
                {
                    m_fAutoChecking = true;
                }

                UINT uRes = ::MessageBox(m_hWnd, strInfo.c_str(), strCaption.c_str(), MB_YESNO);

                if (IDYES == uRes)
                {
                    clock_t tmBegin = ::clock();
                    bool fSuccess = m_dlTask.DownloadToFile(m_UpdateInfo.strFileUrl, m_UpdateInfo.strSavePath, [this, tmBegin](double lfProgress, ULONGLONG ullCur, ULONGLONG ullTotal, ULONGLONG ullSpeed) {

                        if (!m_fQuit)
                        {
                            _tstring strProgress = CStrUtils::Format(_T("已下载: %.2lf%%, %.1lfKB/%.1lfKB, 速度: %.1lfKB/s, 耗时: %.1lf秒"), 
                                (double)ullCur / (double)ullTotal * 100, 
                                (double)ullCur / 1024.0f, (double)ullTotal / 1024.0f, (double)ullSpeed / 1024.0f,
                                (double)(::clock() - tmBegin) / 1000.0f
                            );

                            ::SetWindowText(m_hWnd, strProgress.c_str());
                        }

                        }
                    );

                    if (fSuccess)
                    {
                        UINT fRes = ::MessageBox(m_hWnd, _T("下载完毕, 是否重启软件使用新版本"), _T("更新提示"), MB_YESNO);
                        if (IDYES == fRes)
                        {
                            m_fUpdate = true;
                            PostMessage(WM_CLOSE, 0, 0);
                        }
                    }

                    if (!fManual)
                    {
                        m_fAutoChecking = false;
                    }

                }
                else
                {
                    if (!fManual)
                    {
                        m_fAutoChecking = false;
                    }

                    return false;
                }
            }
            else
            {
                if (fManual)
                {
                    _tstring strCaption = CStrUtils::Format(_T("当前已是最新版本: %s"),
                        m_UpdateInfo.strVersion.c_str());

                    _tstring strInfo = CStrUtils::Format(_T("版本描述: \r\n%s"), strUpdate.c_str());
                    ::MessageBox(m_hWnd, strInfo.c_str(), strCaption.c_str(), MB_OK);
                    return false;
                }
            }
        }
        catch (...)
        {
            OutputDebugString(_T("Some exception...\r\n"));
        }
    }

    ::SetWindowText(m_hWnd, strCaption.c_str());

    return true;
}

LRESULT CMainFrame::OnClose(WPARAM wParam, LPARAM lParam)
{
    if (m_fCuiMode && !m_fCmdShow)
    {
        m_fQuit = false;
        m_wndDraw.WaitTaskQuit();
        return (LRESULT)FALSE;
    }

    m_fQuit = false;
    m_wndDraw.WaitTaskQuit();

    RECT rtFrame = { 0 };
    ::GetWindowRect(m_hWnd, &rtFrame);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_BG_COLOR, m_wndDraw.m_nBgPalIndex);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CH_COLOR, m_wndDraw.m_nChPalIndex);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_COLOR, m_wndDraw.m_nShadePalIndex);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_X, rtFrame.left);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_Y, rtFrame.top);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_START, m_wndDraw.m_fStartChar);


    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_GRID, m_wndDraw.m_fGrid);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_NO_RETURN, m_wndDraw.m_fNoReturn);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_NO_SPACE, m_wndDraw.m_fNoSpace);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_NO_DUPLICATE, m_wndDraw.m_fNoDuplicate);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_ASCII_FULL, m_wndDraw.m_fAsciiFull);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_OUT_WIDTH, m_wndDraw.m_fontOutWidth);
    m_cfg.SetFloat(CONFIG_NAME_ROOT, CONFIG_FONT_SIZE, m_wndDraw.m_fontSize);
    m_cfg.SetString(CONFIG_NAME_ROOT, CONFIG_FONT_NAME, m_wndDraw.m_lf.lfFaceName);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_WIDTH, m_wndDraw.m_fontWidth);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_HEIGHT, m_wndDraw.m_fontHeight);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_ORIENTATION, m_wndDraw.m_fontOrientation);
    
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_START, m_wndDraw.m_fontStart);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_OFFSET_X, m_wndDraw.m_offsetX);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_OFFSET_Y, m_wndDraw.m_offsetY);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_X, m_wndDraw.m_shadeX);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_Y, m_wndDraw.m_shadeY);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_SHADE_ENABLE, m_wndDraw.m_fShade);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_BOLD, m_wndDraw.m_fontBold);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_ITANLIC, m_wndDraw.m_fontItanlic);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_UNDERLINE, m_wndDraw.m_fontUnderLine);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_FONT_STRIKEOUT, m_wndDraw.m_fontStrikeOut);

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_VIEW_SCROLL_POS, m_wndDraw.GetScrollPos());

    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CODE_COMMENT, m_wndDraw.m_fCodeComment);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CODE_X, m_wndDraw.m_fCodeX2);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CODE_Y, m_wndDraw.m_fCodeY2);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CODE_CENTER, m_wndDraw.m_fCodeCenter);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CODE_SIZE, m_wndDraw.m_fCodeSize);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_CODE_MASK, m_wndDraw.m_fCodeMask);

    SCROLLINFO si = { 0 };
    int nBar = SB_VERT;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    ::GetScrollInfo(m_hWndEditText, nBar, &si);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SCROLL_V_POS, si.nPos);

    nBar = SB_HORZ;
    ::GetScrollInfo(m_hWndEditText, nBar, &si);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SCROLL_H_POS, si.nPos);

    DWORD dwStartPos = 0;
    DWORD dwEndPos = 0;
    ::SendMessage(m_hWndEditText, EM_GETSEL, (WPARAM)&dwStartPos, (LPARAM)&dwEndPos);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SEL_START_POS, dwStartPos);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_TEXT_SEL_END_POS, dwEndPos);

    std::string strPathPreFixed = CStrUtils::TStrToAStr(_GetCfgDir() + _T("\\") + CPathUtils::GetCurrentModuleName());
    CStrUtils::ToFileA(CStrUtils::TStrToU8Str(GetItemString(IDC_EDIT_TEXT)), strPathPreFixed + ".txt");
    CStrUtils::ToFileA(CStrUtils::TStrToU8Str(GetItemString(IDC_EDIT_CODE_IN)), strPathPreFixed + "_Input.txt");

    if (m_fUpdate)
    {
        FILE_VERSION_INFO curVersionInfo = CFileVersion::GetCurrentModuleVersionInfo();
        _tstring strTemp = _GetBackName();
        CPathUtils::Rename(CPathUtils::GetCurrentModulePath(), strTemp);
        CPathUtils::Rename(m_UpdateInfo.strSavePath, CPathUtils::GetCurrentModulePath());
        CProcessUtils::RunProcess(CPathUtils::GetCurrentModulePath(), CPathUtils::GetCurrentModuleDir(), true, false);
    }

    return FALSE;
}

LRESULT CMainFrame::OnBgColorChange(WORD wNotify, WORD wID, HWND hWnd)
{
    static DWORD wLastID = m_wndDraw.m_nBgPalIndex + IDC_BUTTON_BG_0;
    m_wndDraw.m_nBgPalIndex = wID - IDC_BUTTON_BG_0;
    m_wndDraw.UpdateSync();
    ::InvalidateRect(GetItem(wLastID), NULL, FALSE);
    wLastID = wID;
    return FALSE;
}

LRESULT CMainFrame::OnChColorChange(WORD wNotify, WORD wID, HWND hWnd)
{
    static DWORD wLastID = m_wndDraw.m_nChPalIndex + IDC_BUTTON_CH_0;
    m_wndDraw.m_nChPalIndex = wID - IDC_BUTTON_CH_0;
    m_wndDraw.UpdateSync();
    ::InvalidateRect(GetItem(wLastID), NULL, FALSE);
    wLastID = wID;
    return FALSE;
}

LRESULT CMainFrame::OnShadeColorChange(WORD wNotify, WORD wID, HWND hWnd)
{
    static DWORD wLastID = m_wndDraw.m_nShadePalIndex + IDC_BUTTON_SHADE_0;
    m_wndDraw.m_nShadePalIndex = wID - IDC_BUTTON_SHADE_0;
    m_wndDraw.UpdateSync();
    ::InvalidateRect(GetItem(wLastID), NULL, FALSE);
    wLastID = wID;
    return FALSE;
}

LRESULT CMainFrame::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
    DWORD dwCtrlIndex = 0;
    DWORD btnColor[4] = {
        RGB(0, 0, 0),
        RGB(99, 207, 99),
        RGB(57, 51, 255),
        RGB(220, 255, 255)
    };

    DWORD* pIndex = &m_wndDraw.m_nShadePalIndex;

    if (lpDrawItem->CtlID >= IDC_BUTTON_BG_0 && lpDrawItem->CtlID <= IDC_BUTTON_BG_3)
    {
        dwCtrlIndex = lpDrawItem->CtlID - IDC_BUTTON_BG_0; 
        pIndex = &m_wndDraw.m_nBgPalIndex;
    }

    if (lpDrawItem->CtlID >= IDC_BUTTON_CH_0 && lpDrawItem->CtlID <= IDC_BUTTON_CH_3)
    {
        dwCtrlIndex = lpDrawItem->CtlID - IDC_BUTTON_CH_0;
        pIndex = &m_wndDraw.m_nChPalIndex;
    }

    if (lpDrawItem->CtlID >= IDC_BUTTON_SHADE_0 && lpDrawItem->CtlID <= IDC_BUTTON_SHADE_3)
    {
        dwCtrlIndex = lpDrawItem->CtlID - IDC_BUTTON_SHADE_0;
        pIndex = &m_wndDraw.m_nShadePalIndex;
    }

    {
        HBRUSH hBkgBrush = (HBRUSH)SendMessage(WM_CTLCOLORDLG, 0, 0);
        HBRUSH hActiveBrush = ::CreateSolidBrush(RGB(255, 0, 0));
        HBRUSH hFocusBrush = ::CreateSolidBrush(RGB(255, 255, 0));
        HBRUSH hColorBrush = ::CreateSolidBrush(btnColor[dwCtrlIndex]);

        ::FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, hBkgBrush);

        if (ODS_FOCUS & lpDrawItem->itemState)
        {
            if (*pIndex == dwCtrlIndex)
            {
                RECT rtDraw = lpDrawItem->rcItem;
                ::FillRect(lpDrawItem->hDC, &rtDraw, hFocusBrush);
                ::InflateRect(&rtDraw, -2, -2);
                ::FillRect(lpDrawItem->hDC, &rtDraw, hColorBrush);
            }
            else
            {
                RECT rtDraw = lpDrawItem->rcItem;
                ::InflateRect(&rtDraw, -2, -2);
                ::FillRect(lpDrawItem->hDC, &rtDraw, hFocusBrush);
                ::InflateRect(&rtDraw, -2, -2);
                ::FillRect(lpDrawItem->hDC, &rtDraw, hColorBrush);
            }
        }
        else
        {
            if (*pIndex == dwCtrlIndex)
            {
                RECT rtDraw = lpDrawItem->rcItem;
                ::FillRect(lpDrawItem->hDC, &rtDraw, hActiveBrush);
                ::InflateRect(&rtDraw, -2, -2);
                ::FillRect(lpDrawItem->hDC, &rtDraw, hColorBrush);
            }
            else
            {
                RECT rtDraw = lpDrawItem->rcItem;
                ::InflateRect(&rtDraw, -2, -2);
                ::InflateRect(&rtDraw, -2, -2);
                ::FillRect(lpDrawItem->hDC, &rtDraw, hColorBrush);
            }
        }

        ::DeleteObject(hColorBrush);
        ::DeleteObject(hFocusBrush);
        ::DeleteObject(hActiveBrush);
    }

    return TRUE;
}

LRESULT CMainFrame::OnCommand(WORD wNotify, WORD wID, HWND hWnd)
{
    return FALSE;
}

LRESULT CMainFrame::OnFontChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnFontStartChange(WORD wNotify, WORD wID, HWND hWnd)
{
    m_wndDraw.m_fontStart = GetItemInt(IDC_EDIT_START);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnTextChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _tstring strText = GetItemString(IDC_EDIT_TEXT);
    m_wndDraw.SetText(strText);

    _tstring strContent = CStrUtils::Format(_T("输入文本 字符: %d/%d (%.3lf%%)"),
        strText.size(), 
        INPUT_TEXT_LENGTH_MAN,
        (double)strText.size() * 100 / (double)INPUT_TEXT_LENGTH_MAN
    );

    SetItemString(IDC_STATIC_INPUT_TEXT, strContent);
    SetItemString(IDC_EDIT_CODE_OUT, m_wndDraw.GetCode(GetItemString(IDC_EDIT_CODE_IN)));

    return FALSE;
}

LRESULT CMainFrame::OnInputTextChange(WORD wNotify, WORD wID, HWND hWnd)
{
    SetItemString(IDC_EDIT_CODE_OUT, m_wndDraw.GetCode(GetItemString(IDC_EDIT_CODE_IN)));
    return FALSE;
}

LRESULT CMainFrame::OnCheckFontCodeComment(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fCodeComment = IsButtonChecked(wID);
    PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
    return TRUE;
}

LRESULT CMainFrame::OnCheckFontCodeX(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fCodeX2 = IsButtonChecked(wID);
    PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
    return TRUE;
}

LRESULT CMainFrame::OnCheckFontCodeY(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fCodeY2 = IsButtonChecked(wID);
    PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
    return TRUE;
}

LRESULT CMainFrame::OnCheckFontCodeCenter(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fCodeCenter = IsButtonChecked(wID);
    PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
    return TRUE;
}

LRESULT CMainFrame::OnCheckFontCodeSize(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fCodeSize = IsButtonChecked(wID);
    PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
    return TRUE;
}

LRESULT CMainFrame::OnCheckFontCodeMask(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fCodeMask = IsButtonChecked(wID);
    PostCommand(IDC_EDIT_CODE_IN, EN_CHANGE);
    return TRUE;
}

LRESULT CMainFrame::OnCheckFontStart(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fStartChar = IsButtonChecked(wID);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnCheckFontAsciiFull(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fAsciiFull = IsButtonChecked(wID);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnCheckFontBold(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fontBold = IsButtonChecked(wID);
    _UpdateFont();

    return FALSE;
}

LRESULT CMainFrame::OnCheckFontItanlic(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fontItanlic = IsButtonChecked(wID);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnCheckFontUnderline(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fontUnderLine = IsButtonChecked(wID);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnCheckFontStrikeOut(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fontStrikeOut = IsButtonChecked(wID);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnCheckFontShade(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fShade = IsButtonChecked(wID);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnOutputMode(WORD wNotify, WORD wID, HWND hWnd)
{
    WORD wLastID = (m_wndDraw.m_eOutMode - eChrOutMode::eOutNormal) + IDC_RADIO_OUT_NORMAL;
    CheckButton(wLastID, FALSE);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_OUTPUT_MODE, wID - IDC_RADIO_OUT_NORMAL);
    m_wndDraw.SetOutMode((eChrOutMode)(wID - IDC_RADIO_OUT_NORMAL));
    CheckButton(wID, TRUE);
    return FALSE;
}

LRESULT CMainFrame::OnTopmost(WORD wNotify, WORD wID, HWND hWnd)
{
    bool fCheck = !IsMenuItemChecked(GetMenu(m_hWnd), wID);
    CheckMenuItem(GetMenu(m_hWnd), wID, fCheck);

    ::SetWindowPos(m_hWnd, fCheck ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_TOPMOST, fCheck);
    return FALSE;
}

LRESULT CMainFrame::OnDebug(WORD wNotify, WORD wID, HWND hWnd)
{
    bool fCheck = !IsMenuItemChecked(GetMenu(m_hWnd), wID);
    CheckMenuItem(GetMenu(m_hWnd), wID, fCheck);

    m_wndDraw.m_fDebug = fCheck;
    _UpdateFont();
    m_cfg.SetInt(CONFIG_NAME_ROOT, CONFIG_WINDOW_DEBUG, fCheck);
    return FALSE;
}

LRESULT CMainFrame::OnQuit(WORD wNotify, WORD wID, HWND hWnd)
{
    PostMessage(WM_CLOSE, 0, 0);
    return FALSE;
}

LRESULT CMainFrame::OnAbout(WORD wNotify, WORD wID, HWND hWnd)
{
    ShellExecute(NULL, _T("open"), HELP_ABOUT_URL, NULL, NULL, SW_SHOW);
    return FALSE;
}

LRESULT CMainFrame::OnUpdate(WORD wNotify, WORD wID, HWND hWnd)
{
    static bool fChecking = false;

    if (fChecking)
    {
        return FALSE;
    }

    fChecking = true;
    std::thread([this]() {
        _UpdateCheck(true);
        fChecking = false;
        }).detach();

    return FALSE;
}

LRESULT CMainFrame::OnFont_8_8(WORD wNotify, WORD wID, HWND hWnd)
{
    m_wndDraw.m_fontSize = 7;
    m_wndDraw.m_fontWidth = 8;
    m_wndDraw.m_fontHeight = 8;
    m_wndDraw.m_offsetX = 0;
    m_wndDraw.m_offsetY = -1;
    SetItemFloat(IDC_EDIT_OFFSET_X, m_wndDraw.m_offsetX);
    SetItemFloat(IDC_EDIT_OFFSET_Y, m_wndDraw.m_offsetY);
    SetItemFloat(IDC_EDIT_FONT_SIZE, m_wndDraw.m_fontSize);
    SetItemFloat(IDC_EDIT_FONT_WIDTH, m_wndDraw.m_fontWidth);
    SetItemFloat(IDC_EDIT_FONT_HEIGHT, m_wndDraw.m_fontHeight);
    _UpdateFont();
    return TRUE;
}

LRESULT CMainFrame::OnFont_12_8(WORD wNotify, WORD wID, HWND hWnd)
{
    m_wndDraw.m_fontSize = 9;
    m_wndDraw.m_fontWidth = 12;
    m_wndDraw.m_fontHeight = 16;
    m_wndDraw.m_offsetX = 0;
    SetItemFloat(IDC_EDIT_OFFSET_X, m_wndDraw.m_offsetX);
    SetItemFloat(IDC_EDIT_OFFSET_Y, m_wndDraw.m_offsetY);
    SetItemFloat(IDC_EDIT_FONT_SIZE, m_wndDraw.m_fontSize);
    SetItemFloat(IDC_EDIT_FONT_WIDTH, m_wndDraw.m_fontWidth);
    SetItemFloat(IDC_EDIT_FONT_HEIGHT, m_wndDraw.m_fontHeight);
    _UpdateFont();
    return TRUE;
}

LRESULT CMainFrame::OnFont_12_12(WORD wNotify, WORD wID, HWND hWnd)
{
    m_wndDraw.m_fontSize = 9;
    m_wndDraw.m_fontWidth = 12;
    m_wndDraw.m_fontHeight = 16;
    m_wndDraw.m_offsetX = 0;
    m_wndDraw.m_offsetY = 0;
    SetItemFloat(IDC_EDIT_OFFSET_X, m_wndDraw.m_offsetX);
    SetItemFloat(IDC_EDIT_OFFSET_Y, m_wndDraw.m_offsetY);
    SetItemFloat(IDC_EDIT_FONT_SIZE, m_wndDraw.m_fontSize);
    SetItemFloat(IDC_EDIT_FONT_WIDTH, m_wndDraw.m_fontWidth);
    SetItemFloat(IDC_EDIT_FONT_HEIGHT, m_wndDraw.m_fontHeight);
    _UpdateFont();
    return TRUE;
}

LRESULT CMainFrame::OnFont_16_16(WORD wNotify, WORD wID, HWND hWnd)
{
    m_wndDraw.m_fontSize = 13;
    m_wndDraw.m_fontWidth = 16;
    m_wndDraw.m_fontHeight = 16;
    m_wndDraw.m_offsetX = 0;
    m_wndDraw.m_offsetY = -1;
    SetItemFloat(IDC_EDIT_OFFSET_X, m_wndDraw.m_offsetX);
    SetItemFloat(IDC_EDIT_OFFSET_Y, m_wndDraw.m_offsetY);
    SetItemFloat(IDC_EDIT_FONT_SIZE, m_wndDraw.m_fontSize);
    SetItemFloat(IDC_EDIT_FONT_WIDTH, m_wndDraw.m_fontWidth);
    SetItemFloat(IDC_EDIT_FONT_HEIGHT, m_wndDraw.m_fontHeight);
    _UpdateFont();
    return TRUE;
}

LRESULT CMainFrame::OnGrid(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fGrid = IsButtonChecked(wID);
    m_wndDraw.UpdateSync();
    return FALSE;
}

LRESULT CMainFrame::OnNoReturn(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fNoReturn = IsButtonChecked(wID);
    m_wndDraw.UpdateText();
    return FALSE;
}

LRESULT CMainFrame::OnNoSpace(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fNoSpace = IsButtonChecked(wID);
    m_wndDraw.UpdateText();
    return FALSE;
}

LRESULT CMainFrame::OnNoDuplicate(WORD wNotify, WORD wID, HWND hWnd)
{
    if (wNotify)
    {
        CheckButton(wID, !IsButtonChecked(wID));
    }

    m_wndDraw.m_fNoDuplicate = IsButtonChecked(wID);
    m_wndDraw.UpdateText();
    return FALSE;
}

static POINT GetChildWindowCenterPos(
    int nWidth,         //子窗口宽度
    int nHeight,        //子窗口高度
    HWND hParent        //父窗口句柄
)
{
    RECT rectParent = { 0 };
    LONG nParentW = 0;
    LONG nParentH = 0;

    // 如果父窗口句柄为空, 则以桌面窗口为父窗口
    if (NULL == hParent)
    {
        hParent = ::GetDesktopWindow();
    }

    // 获取并统计父窗口宽度和高度
    ::GetWindowRect(hParent, &rectParent);
    nParentW = rectParent.right - rectParent.left;
    nParentH = rectParent.bottom - rectParent.top;

    return { rectParent.left + (nParentW - nWidth) / 2, rectParent.top + (nParentH - nHeight) / 2 };
}

LRESULT CMainFrame::OnExportChr(WORD wNotify, WORD wID, HWND hWnd)
{
    if (m_fBusy)
    {
        return FALSE;
    }

    m_fBusy = true;

    std::thread([this]() {
        COMDLG_FILTERSPEC filters[] = {
            { L"CHR", L"*.chr" },
            { L"NES", L"*.nes" }
        };

        static _tstring strPath;
        static UINT uIndex = 0;

        if (strPath.empty())
        {
            strPath = m_cfg.GetString(CONFIG_NAME_ROOT, CONFIG_OUTPUT_PATH, _T(""));

            _tstring strDir = CPathUtils::GetFileDir(strPath);
            _tstring strName = CPathUtils::GetFileName(strPath, true);
            _tstring strExt = CPathUtils::GetFileExt(strPath, false);

            if (0 == _tcsicmp(strExt.c_str(), _T("chr")))
            {
                uIndex = 1;
            }

            if (0 == _tcsicmp(strExt.c_str(), _T("nes")))
            {
                uIndex = 2;
            }

            if (strName.empty())
            {
                strName = _T("out");
            }

            if (!CPathUtils::IsExist(strDir))
            {
                strPath = CPathUtils::GetCurrentModuleDir();
                strPath += _T("\\");
                strPath += strName;
            }
        }

        if (CComFileDlg::GetSavePath(m_hWnd, strPath, uIndex, filters, _countof(filters)))
        {
            if (1 == uIndex && 0 != _tcsicmp(CPathUtils::GetFileExt(strPath).c_str(), _T("chr")))
            {
                strPath += _T(".chr");
            }

            if (2 == uIndex && 0 != _tcsicmp(CPathUtils::GetFileExt(strPath).c_str(), _T("nes")))
            {
                strPath += _T(".nes");
            }

            m_cfg.SetString(CONFIG_NAME_ROOT, CONFIG_OUTPUT_PATH, strPath);

            // 导出
            TCHAR szBuf[MAX_PATH] = {0};
            ::GetWindowText(m_hWnd, szBuf, _countof(szBuf));
            clock_t timeBegin = ::clock();
            int nTotalCount = 0;
            m_wndDraw.ExportChr(strPath, [this, &nTotalCount](double progress,
                size_t cur,
                size_t total,
                double speed,
                double costTime,
                double remainingTime) {
                
                nTotalCount = total;
                static _tstring strProgress;
                
                strProgress = CStrUtils::Format(_T("进度: %.1lf%% %zu/%zu 速度: %.0lf/s 耗时: %.1lfs 剩余: %.1lfs"),
                    (double)(cur * 100) / (double)total,
                    cur, total,
                    speed,
                    costTime,
                    remainingTime
                    );

                ::SetWindowText(m_hWnd, strProgress.c_str());
                return !m_fQuit;

                }
            );
            clock_t timeEnd = ::clock();
            ::SetWindowText(m_hWnd, szBuf);

            _tstring strMessage = CStrUtils::Format(_T("保存文件成功, 耗时: %.3f秒, 共计 %zu 个字模, 平均速度: %.3lf/s\r\n是否打开输出位置?"), 
                (double)(timeEnd - timeBegin) / 1000.0f, nTotalCount, (double)nTotalCount / ((double)(timeEnd - timeBegin) / 1000.0f)
            );

            if (!m_fQuit)
            {
                static HHOOK g_hHook = NULL;
                DWORD dwTid = ::GetCurrentThreadId();
                g_hHook = ::SetWindowsHookEx(WH_CBT, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
                    if (nCode < HC_ACTION)
                    {
                        return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
                    }

                    // 即将创建一个窗口
                    /*if (HCBT_ACTIVATE == nCode)
                    {
                        HWND hWndActivate = (HWND)wParam;
                        LPCBTACTIVATESTRUCT pStruct = (LPCBTACTIVATESTRUCT)lParam;

                        if (32770 == ::GetClassLongPtr(hWndActivate, GCW_ATOM))
                        {
                            HANDLE lFlag = ::GetProp(hWndActivate, _T("Center"));
                            if (nullptr == lFlag)
                            {
                                RECT rtWnd = { 0 };
                                ::GetWindowRect(hWndActivate, &rtWnd);
                                POINT nPos = GetChildWindowCenterPos(rtWnd.right - rtWnd.left, rtWnd.bottom - rtWnd.top, ::GetParent(hWndActivate));
                                ::MoveWindow(hWndActivate, nPos.x, nPos.y, rtWnd.right - rtWnd.left, rtWnd.bottom - rtWnd.top, true);

                                ::SetProp(hWndActivate, _T("Center"), (HANDLE)1);
                            }
                        }
                    }*/

                    if (HCBT_CREATEWND  == nCode)
                    {
                        HWND hWndNew = (HWND)wParam;
                        LPCBT_CREATEWND pStruct = (LPCBT_CREATEWND)lParam;
                        HWND hParent = pStruct->lpcs->hwndParent;

                        if (32770 == ::GetClassLongPtr(hWndNew, GCW_ATOM))
                        {
                            POINT nPos = GetChildWindowCenterPos(pStruct->lpcs->cx, pStruct->lpcs->cy, hParent);
                            pStruct->lpcs->x = nPos.x;
                            pStruct->lpcs->y = nPos.y;
                        }
                    }

                    return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
                    
                    }, NULL, dwTid);

                if (IDYES == ::MessageBox(m_hWnd, strMessage.c_str(), _T("操作完成"), MB_YESNO | MB_ICONINFORMATION))
                {
                    _tstring strDir = CPathUtils::GetFileDir(strPath);
                    _tstring strCmd = _T("\"explorer\" /select, \"") + strPath + _T('\"');
                    CProcessUtils::RunProcess(strCmd, strDir, true, false);
                }
                ::UnhookWindowsHookEx(g_hHook);
            }

        }

        m_fBusy = false;
        }).detach();

    return TRUE;
}

void CMainFrame::_LimitRange(WORD wEditID, WORD wSpinID, double& nVal, double nMin, double nMax)
{
    nVal = GetItemFloat(wEditID);

    if (nVal < nMin)
    {
        nVal = nMin;
        SetItemFloat(wEditID, nVal);
    }

    if (nVal > nMax)
    {
        nVal = nMax;
        SetItemFloat(wEditID, nVal);
    }

    SendItemMessage(wSpinID, UDM_SETPOS32, 0, nVal);

}

void CMainFrame::_LimitRange(WORD wEditID, WORD wSpinID, int& nVal, int nMin, int nMax)
{
    nVal = GetItemInt(wEditID);

    if (nVal < nMin)
    {
        nVal = nMin;
        SetItemInt(wEditID, nVal);
    }

    if (nVal > nMax)
    {
        nVal = nMax;
        SetItemInt(wEditID, nVal);
    }

    SendItemMessage(wSpinID, UDM_SETPOS32, 0, nVal);

}

LRESULT CMainFrame::OnFontSizeChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_FONT_SIZE, IDC_SPIN_FONT_SIZE, m_wndDraw.m_fontSize, 1.0f, 128.0f);
    _UpdateFont();

    return FALSE;
}

LRESULT CMainFrame::OnFontWidthChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_FONT_WIDTH, IDC_SPIN_FONT_WIDTH, m_wndDraw.m_fontWidth, 1, 128);
    _LimitRange(IDC_EDIT_OFFSET_X, IDC_SPIN_OFFSET_X, m_wndDraw.m_offsetX, -m_wndDraw.m_fontWidth, m_wndDraw.m_fontWidth);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnFontOutWidthChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_FONT_OUT_WIDTH, IDC_SPIN_FONT_OUT_WIDTH, m_wndDraw.m_fontOutWidth, 1, 256);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnFontHeightChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_FONT_HEIGHT, IDC_SPIN_FONT_HEIGHT, m_wndDraw.m_fontHeight, 1, 128);
    _LimitRange(IDC_EDIT_OFFSET_Y, IDC_SPIN_OFFSET_Y, m_wndDraw.m_offsetY, -m_wndDraw.m_fontHeight, m_wndDraw.m_fontHeight);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnFontOrientationChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_FONT_ORIENTATION, IDC_SPIN_FONT_ORIENTATION, m_wndDraw.m_fontOrientation, -180, 180);
    _UpdateFont();
    return FALSE;
}

LRESULT CMainFrame::OnFontOffsetXChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_OFFSET_X, IDC_SPIN_OFFSET_X, m_wndDraw.m_offsetX, -m_wndDraw.m_fontWidth, m_wndDraw.m_fontWidth);
    m_wndDraw.UpdateSync();
    return FALSE;
}

LRESULT CMainFrame::OnFontOffsetYChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_OFFSET_Y, IDC_SPIN_OFFSET_Y, m_wndDraw.m_offsetY, -m_wndDraw.m_fontHeight, m_wndDraw.m_fontHeight);
    m_wndDraw.UpdateSync();
    return FALSE;
}

LRESULT CMainFrame::OnFontShadeXChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_SHADE_X, IDC_SPIN_SHADE_X, m_wndDraw.m_shadeX, -32, 32);
    m_wndDraw.UpdateSync();
    return FALSE;
}

LRESULT CMainFrame::OnFontShadeYChange(WORD wNotify, WORD wID, HWND hWnd)
{
    _LimitRange(IDC_EDIT_SHADE_Y, IDC_SPIN_SHADE_Y, m_wndDraw.m_shadeY, -32, 32);
    m_wndDraw.UpdateSync();
    return FALSE;
}

LRESULT CMainFrame::OnNotifyFontSize(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;

    double fVal = GetItemFloat(IDC_EDIT_FONT_SIZE);
    double fStep = (double)(lpNmhdr->iDelta) * 0.5f;
    fVal = fVal + fStep;

    SetItemFloat(IDC_EDIT_FONT_SIZE, fVal);

    _LimitRange(IDC_EDIT_FONT_SIZE, IDC_SPIN_FONT_SIZE, m_wndDraw.m_fontSize, 1.0f, 128.0f);
    _UpdateFont();

    return TRUE;
}

LRESULT CMainFrame::OnNotifyFontWidth(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    if (lpNmhdr->iPos + lpNmhdr->iDelta < 1)
    {
        SetItemInt(IDC_EDIT_FONT_WIDTH, 1);
    }
    else
    {
        SetItemInt(IDC_EDIT_FONT_WIDTH, lpNmhdr->iPos + lpNmhdr->iDelta);
    }
    _LimitRange(IDC_EDIT_FONT_WIDTH, IDC_SPIN_FONT_WIDTH, m_wndDraw.m_fontWidth, 1, 128);
    _LimitRange(IDC_EDIT_OFFSET_X, IDC_SPIN_OFFSET_X, m_wndDraw.m_offsetX, -m_wndDraw.m_fontWidth, m_wndDraw.m_fontWidth);
    _UpdateFont();

    return TRUE;
}

LRESULT CMainFrame::OnNotifyFontOutWidth(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    if (lpNmhdr->iPos + lpNmhdr->iDelta < 1)
    {
        SetItemInt(IDC_EDIT_FONT_OUT_WIDTH, 1);
    }
    else
    {
        SetItemInt(IDC_EDIT_FONT_OUT_WIDTH, lpNmhdr->iPos + lpNmhdr->iDelta);
    }
    _LimitRange(IDC_EDIT_FONT_OUT_WIDTH, IDC_SPIN_FONT_OUT_WIDTH, m_wndDraw.m_fontOutWidth, 1, 256);
    _UpdateFont();

    return TRUE;
}

LRESULT CMainFrame::OnNotifyFontHeight(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    if (lpNmhdr->iPos + lpNmhdr->iDelta < 1)
    {
        SetItemInt(IDC_EDIT_FONT_HEIGHT, 1);
    }
    else
    {
        SetItemInt(IDC_EDIT_FONT_HEIGHT, lpNmhdr->iPos + lpNmhdr->iDelta);
    }
    _LimitRange(IDC_EDIT_FONT_HEIGHT, IDC_SPIN_FONT_HEIGHT, m_wndDraw.m_fontHeight, 1, 128);
    _LimitRange(IDC_EDIT_OFFSET_Y, IDC_SPIN_OFFSET_Y, m_wndDraw.m_offsetY, -m_wndDraw.m_fontHeight, m_wndDraw.m_fontHeight);

    _UpdateFont();

    return TRUE;
}

LRESULT CMainFrame::OnNotifyFontOrientation(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    SetItemInt(IDC_EDIT_FONT_ORIENTATION, lpNmhdr->iPos + lpNmhdr->iDelta);
    _LimitRange(IDC_EDIT_FONT_ORIENTATION, IDC_SPIN_FONT_ORIENTATION, m_wndDraw.m_fontOrientation, -180, 180);
    _UpdateFont();
    return TRUE;
}

LRESULT CMainFrame::OnNotifyOffsetXChange(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    SetItemInt(IDC_EDIT_OFFSET_X, lpNmhdr->iPos + lpNmhdr->iDelta);
    _LimitRange(IDC_EDIT_OFFSET_X, IDC_SPIN_OFFSET_X, m_wndDraw.m_offsetX, -m_wndDraw.m_fontWidth, m_wndDraw.m_fontWidth);
    m_wndDraw.UpdateSync();
    return TRUE;
}

LRESULT CMainFrame::OnNotifyOffsetYChange(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    SetItemInt(IDC_EDIT_OFFSET_Y, lpNmhdr->iPos + lpNmhdr->iDelta);
    _LimitRange(IDC_EDIT_OFFSET_Y, IDC_SPIN_OFFSET_Y, m_wndDraw.m_offsetY, -m_wndDraw.m_fontHeight, m_wndDraw.m_fontHeight);
    m_wndDraw.UpdateSync();
    return TRUE;
}

LRESULT CMainFrame::OnNotifyShadeXChange(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    SetItemInt(IDC_EDIT_SHADE_X, lpNmhdr->iPos + lpNmhdr->iDelta);
    _LimitRange(IDC_EDIT_SHADE_X, IDC_SPIN_SHADE_X, m_wndDraw.m_shadeX, -128, 128);
    m_wndDraw.UpdateSync();
    return TRUE;
}

LRESULT CMainFrame::OnNotifyShadeYChange(WORD wID, LPNMHDR pNmhdr)
{
    LPNMUPDOWN lpNmhdr = (LPNMUPDOWN)pNmhdr;
    SetItemInt(IDC_EDIT_SHADE_Y, lpNmhdr->iPos + lpNmhdr->iDelta);
    _LimitRange(IDC_EDIT_SHADE_Y, IDC_SPIN_SHADE_Y, m_wndDraw.m_shadeY, -128, 128);
    m_wndDraw.UpdateSync();
    return TRUE;
}

void CMainFrame::_UpdateFont()
{
    int nSel = ComboBox_GetCurSel(m_hWndFontList);

    if (nSel > -1)
    {
        LOGFONT lf = m_vLogFonts[nSel];
        lf.lfWidth = 0;
        lf.lfQuality = NONANTIALIASED_QUALITY;
        lf.lfItalic = m_wndDraw.m_fontItanlic;
        lf.lfWeight = m_wndDraw.m_fontBold ? FW_BOLD : 0;
        lf.lfUnderline = m_wndDraw.m_fontUnderLine;
        lf.lfStrikeOut = m_wndDraw.m_fontStrikeOut;

        m_wndDraw.SetFont(&lf);
    }
}

BOOL CMainFrame::PreTranslateMessage(LPMSG pMsg)
{
    // 繁忙时段不处理客户区鼠标消息
    if (m_fBusy)
    {
        if (pMsg->message >= WM_LBUTTONDOWN && pMsg->message <= WM_XBUTTONDBLCLK)
        {
            return TRUE;
        }
    }

    if (WM_DROPFILES == pMsg->message)
    {
        HDROP hDrop = (HDROP)pMsg->wParam;
        TCHAR szFile[MAX_PATH] = { 0 };

        UINT nCount = DragQueryFile(hDrop, (UINT)(-1), szFile, _countof(szFile));
        for (UINT i = 0; i < nCount; i++)
        {
            ::DragQueryFile(hDrop, i, szFile, _countof(szFile));

            if (CPathUtils::IsArchive(szFile))
            {
                std::vector<uint8_t> vData;
                if (!CBytesUtils::BytesFromFile(vData, szFile))
                {
                    break;
                }

                int nNullCount = 0;
                for (const auto& item: vData)
                {
                    if (0 == item)
                    {
                        nNullCount++;
                    }
                }

                vData.push_back('\0');
                vData.push_back('\0');

                if (nNullCount > 1)
                {
                    _tstring strContent = (LPCTSTR)vData.data();
                    SetItemString(IDC_EDIT_TEXT, strContent);
                    m_wndDraw.SetText(strContent);
                }
                else
                {
                    std::string strText = (const char*)vData.data();

                    _tstring strContent;
                    if (CStrUtils::GetUtf8Count(strText.c_str()))
                    {
                        strContent = CStrUtils::U8StrToTStr(strText);
                    }
                    else
                    {
                        strContent = CStrUtils::AStrToTStr(strText);
                    }

                    SetItemString(IDC_EDIT_TEXT, strContent);
                    m_wndDraw.SetText(strContent);
                }

                OnTextChange(0, 0, 0);
            }

            break;
        }

        ::DragFinish(hDrop);
    }

    return FALSE;
}