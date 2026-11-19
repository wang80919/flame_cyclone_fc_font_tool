#pragma once

#define OEMRESOURCE

#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <string>
#include <commctrl.h>
#include <vector>

#pragma warning(disable:4100)

#define WM_MDI_CHILD_CREATE         (WM_USER + 0x200)
#define WM_MDI_CHILD_DESTROY        (WM_USER + 0x201)
#define WM_MDI_MESSAGE              (WM_USER + 0x202)
#define WM_MDI_CHILD_MESSAGE        (WM_USER + 0x203)

#ifdef UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CWindowBase;

typedef LRESULT(CWindowBase::* pWndMessageFn)(WPARAM wParam, LPARAM lParam);
typedef LRESULT(CWindowBase::* pWndCommandFn)(WORD wNotify, WORD wID, HWND hWnd);
typedef LRESULT(CWindowBase::* pWndNotifyFn)(WORD wID, LPNMHDR pNmhdr);

// 多文档消息数据
typedef struct _MDI_MESSAGE_DATA
{
    HWND hWnd;
    WPARAM wParam;
    LPARAM lParam;
    DWORD dwID;

    _MDI_MESSAGE_DATA()
    {
        hWnd = NULL;
        wParam = 0;
        lParam = 0;
        dwID = 0;
    }

}MDI_MESSAGE_DATA, *LPMDI_MESSAGE_DATA;

struct WND_MSGMAP_ENTRY
{
    union
    {
        pWndMessageFn m_pFnMessage;
        pWndCommandFn m_pFnCommand;
        pWndNotifyFn m_pFnNotify;

    }m_pFunc;               //要调用的例程（或特殊值）
    UINT m_nMessage;        //Windows消息
    UINT m_nCode;           //控件码或 or WM_NOTIFY code
    UINT m_nID;             //控件ID (Windows消息则为0)
    UINT m_nLastID;         //用于指定控件 ID 范围的条目

    WND_MSGMAP_ENTRY()
        :
        m_nMessage(0),
        m_nCode(0),
        m_nID(0),
        m_nLastID(0)
    {
        m_pFunc.m_pFnMessage = nullptr;
    }

    WND_MSGMAP_ENTRY(UINT uMsg, pWndMessageFn pfn)
        : 
        m_nMessage(uMsg), 
        m_nCode(0),
        m_nID(0),
        m_nLastID(0)
    {
        m_pFunc.m_pFnMessage = pfn;
    }

    WND_MSGMAP_ENTRY(UINT uMsg, UINT id, pWndCommandFn pfn)
        :
        m_nMessage(uMsg),
        m_nCode(0),
        m_nID(id),
        m_nLastID(0)
    {
        m_pFunc.m_pFnCommand = pfn;
    }

    WND_MSGMAP_ENTRY(UINT uMsg, UINT id, UINT idLast, pWndCommandFn pfn)
        :
        m_nMessage(uMsg),
        m_nCode(0),
        m_nID(id),
        m_nLastID(idLast)
    {
        m_pFunc.m_pFnCommand = pfn;
    }

    WND_MSGMAP_ENTRY(UINT uMsg, UINT wNotifyCode, UINT id, UINT idLast, pWndNotifyFn pfn)
        :
        m_nMessage(uMsg),
        m_nCode(wNotifyCode),
        m_nID(id),
        m_nLastID(idLast)
    {
        m_pFunc.m_pFnNotify = pfn;
    }
};

struct WND_MSGMAP
{
    const WND_MSGMAP* (*pfnGetBaseMap)();
    const WND_MSGMAP_ENTRY* lpEntries;
};

#ifndef wnd_msg
#define wnd_msg
#endif

#ifndef wnd_command
#define wnd_command
#endif

#ifndef wnd_notify
#define wnd_notify
#endif

#define DECLARE_WND_MESSAGE_MAP()                               \
public:                                                         \
operator HWND() const { return m_hWnd; }                        \
protected:                                                      \
static const WND_MSGMAP* GetThisMessageMap();                   \
virtual const WND_MSGMAP* GetMessageMap() const;                \

#define BEGIN_WND_MESSAGE_MAP(theClass, baseClass)              \
const WND_MSGMAP* theClass::GetMessageMap() const               \
{                                                               \
    return GetThisMessageMap();                                 \
}                                                               \
const WND_MSGMAP* theClass::GetThisMessageMap()                 \
{                                                               \
    typedef baseClass TheBaseClass;                                \
static const WND_MSGMAP_ENTRY _messageEntries[] =               \
{                                                               \
 
#define ON_WND_MESSAGE(_msg, _Fn)                               \
WND_MSGMAP_ENTRY(_msg, static_cast< LRESULT (CWindowBase::*)(WPARAM, LPARAM) >(_Fn)),

#define ON_WND_COMMAND(_id, _Fn)                                \
WND_MSGMAP_ENTRY(WM_COMMAND, _id, static_cast< LRESULT (CWindowBase::*)(WORD wNotify, WORD wID, HWND hWnd) >(_Fn)),

#define ON_WND_COMMAND_RANGE(_id, _idLast, _Fn)                 \
WND_MSGMAP_ENTRY(WM_COMMAND, _id, _idLast, static_cast< LRESULT (CWindowBase::*)(WORD wNotify, WORD wID, HWND hWnd) >(_Fn)),

#define ON_WND_NOTIFY(_wNotifycode, _id, _Fn)                   \
WND_MSGMAP_ENTRY(WM_NOTIFY, (UINT)_wNotifycode, _id, _id,  static_cast< LRESULT (CWindowBase::*)(WORD wID, LPNMHDR pNmhdr) >(_Fn)),

#define ON_WND_NOTIFY_RANGE(_wNotifycode, _id, _idLast, _Fn)    \
WND_MSGMAP_ENTRY(WM_NOTIFY, (UINT)_wNotifycode, _id, _idLast,  static_cast< LRESULT (CWindowBase::*)(WORD wID, LPNMHDR pNmhdr) >(_Fn)),

#define END_WND_MESSAGE_MAP()                                   \
WND_MSGMAP_ENTRY(0, nullptr) };                                 \
static const WND_MSGMAP messageMap =                            \
{&TheBaseClass::GetThisMessageMap ,_messageEntries };      \
    return &messageMap;                                         \
}                                                               \

#define DECLARE_WND_BASE_MESSAGE_MAP()                          \
protected:                                                      \
static const WND_MSGMAP* GetThisMessageMap();                   \
virtual const WND_MSGMAP* GetMessageMap() const;                \
static const WND_MSGMAP* GetBaseMessageMap();                   \

#define BEGIN_WND_BASE_MESSAGE_MAP(theClass)                    \
typedef theClass TheThisClass;                                    \
const WND_MSGMAP* TheThisClass::GetMessageMap() const           \
{                                                               \
    return GetThisMessageMap();                                 \
}                                                               \
const WND_MSGMAP* TheThisClass::GetBaseMessageMap()             \
{                                                               \
    return NULL;                                                \
}                                                               \
const WND_MSGMAP* TheThisClass::GetThisMessageMap()             \
{                                                               \
static const WND_MSGMAP_ENTRY _messageEntries[] =               \
{                                                               \

#define END_WND_BASE_MESSAGE_MAP()                              \
WND_MSGMAP_ENTRY(0, nullptr) };                                 \
static const WND_MSGMAP messageMap =                            \
{&TheThisClass::GetBaseMessageMap, &_messageEntries[0] };       \
return &messageMap;                                             \
}                                                               \

typedef struct _SUBCLASS_WND_INFO
{
    HWND hWnd = NULL;
    UINT_PTR uID = (UINT_PTR)(-1);

    _SUBCLASS_WND_INFO(HWND hWnd, UINT_PTR uID)
    {
        this->hWnd = hWnd;
        this->uID = uID;
    }

}SUBCLASS_WND_INFO;

class CWindowBase
{
public:

    CWindowBase();
    CWindowBase(const CWindowBase& r) = delete;
    CWindowBase& operator = (const CWindowBase& r) = delete;
    virtual ~CWindowBase();

    // 注册窗口类
    virtual BOOL Register(_tstring strClassName);

    // 超类化窗口
    virtual BOOL Superclass(_tstring strTargetClass, _tstring strClassName);

    // 子类化窗口
    virtual BOOL Subclass(HWND hWnd, UINT_PTR uId);
    BOOL RemoveSubclass();

    // 创建窗口
    virtual BOOL Create(
        HWND hParent,
        HMENU hMenu,
        _tstring strTitle,
        int nCmdShow = SW_SHOW,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        DWORD dwExStyle = 0,
        int dwX = 0,
        int dwY = 0,
        int dwWidth = 640,
        int dwHeight = 480,
        bool bCenter = true
    );

    // 执行消息环
    UINT_PTR Run();

    virtual LRESULT OnCommand(WORD wNotify, WORD wID, HWND hWnd);
    virtual LRESULT OnNotify(WORD wID, LPNMHDR pNmhdr);

    virtual LRESULT OnDestroy();

    // 多文档子窗口消息
    virtual LRESULT OnMDIMessage(LPMDI_MESSAGE_DATA lpMsg);
    virtual LRESULT OnChildMessage(LPMDI_MESSAGE_DATA lpMsg);

    virtual LRESULT OnAfterStatusBarShow(bool bShow);
    virtual LRESULT OnAfterToolBarShow(bool bShow);

    // 获取窗口句柄
    HWND GetWndHandle() const;

public:

    // 消息
    bool PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam = 0);
    LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam = 0);
    bool PostCommand(WORD wID, WORD wNotify = 0, LPARAM lParam = 0);
    LRESULT SendCommand(WORD wID, WORD wNotify = 0, LPARAM lParam = 0);
    LRESULT SendMDIMessage(const LPMDI_MESSAGE_DATA lpMsg);

    // 显示
    bool SetIcon(DWORD dwId);
    void ShowWindow(int cmdShow = SW_SHOW);
    bool IsVisible() const;

    void MoveToCenter(HWND hParent = NULL);

    // 加速键
    HACCEL GetAccel() const;
    bool LoadAccel(DWORD dwId);

    _tstring LoadString(UINT resourceID);

    // 子窗口管理
    _tstring GetItemString(DWORD dwID);
    void SetItemString(DWORD dwID, const _tstring& strText);
    bool IsButtonChecked(int nIDButton) const;
    bool CheckButton(int nIDButton, bool bCheck = true);
    HWND GetItem(DWORD dwID) const;

    // 工具栏管理
    HWND GetToolBar();
    bool IsToolBarVisible();
    bool ToolBarCreate();
    LRESULT ToolBarSetImageList(int id, HIMAGELIST hImgList);
    LRESULT ToolBarLoadImages(DWORD dwFlag);
    LRESULT ToolBarSetButtonStructSize(size_t nTbButtonSize);
    LRESULT ToolBarAddButtons(int numButtons, LPTBBUTTON* lpTbButton);
    LRESULT ToolBarAddBitmap(int numButtons, LPTBADDBITMAP lpTbButton);
    LRESULT ToolBarAutoSize();
    LRESULT ToolBarShow(bool bShow = true);
    LRESULT ToolBarButtonEnable(DWORD dwButtonID, bool fEnable);
    LRESULT ToolBarButtonShow(DWORD dwButtonID, bool fEnable);

    // 菜单项管理
    HMENU GetMenu();
    bool SetMenu(HMENU hMenu);
    bool DrawMenuBar();
    void MenuItemCheck(UINT uId, bool bCheck, bool fByPosition = false);
    bool MenuItemIsChecked(UINT uId, bool fByPosition = false);
    bool MenuItemIsEnable(UINT uId, bool fByPosition = false);
    void MenuItemEnable(UINT uId, bool fEnable, bool fByPosition = false);
    bool SetMenuItemText(UINT uId, const _tstring strText, bool fByPosition = false);
    bool SetMenuItemText(UINT uId, UINT resourceID, bool fByPosition = false);
    _tstring GetMenuItemText(UINT uId, bool bCheck, bool fByPosition = false);

    // 状态栏管理
    HWND GetStatusBar();
    bool IsStatusBarVisible();
    bool StatusBarCreate();
    bool StatusBarSetParts(std::vector<int> vParts = {});
    bool StatusBarSetParts(int nParts);
    bool StatusBarSetText(int nPart, _tstring strContent);
    bool StatusBarAutoSize();
    void StatusBarShow(bool bShow = true);

protected:

    wnd_msg LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnEraseBkgnd(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

    wnd_msg LRESULT OnMDIMessage(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnChildMessage(WPARAM wParam, LPARAM lParam);

    wnd_command LRESULT OnCommandDemo(WORD wNotify, WORD wID, HWND hWnd);
    wnd_notify LRESULT OnNotifyDemo(WORD wID, LPNMHDR pNmhdr);

    virtual BOOL PreTranslateMessage(LPMSG pMsg);

    bool BuildDcBitmap(HDC hdc, HDC* pDC, HBITMAP* pBitmap, int nW, int nH, DWORD dwColor = RGB(128,128,128));
    void DeleteDcBitmap(HDC* pDC, HBITMAP* pBitmap);

private:

    static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    LRESULT ThisWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    _tstring    m_ClassName;
    HWND        m_hWnd;
    BOOL        m_bCenter;
    WNDPROC     m_SuperProc = nullptr;
    UINT_PTR    m_uIdSubclass = (UINT_PTR)(-1);

private:
    HWND        m_hToolBar;                         // 工具栏
    HWND        m_hStatusBar;                       // 状态栏
    HACCEL      m_hAccel;                           // 快捷键
    HMENU       m_hMenu ;                           // 菜单

    DECLARE_WND_BASE_MESSAGE_MAP()
};