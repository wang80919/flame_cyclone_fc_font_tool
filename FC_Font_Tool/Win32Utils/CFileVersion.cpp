#include "CFileVersion.h"
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "Version.lib")

#pragma pack(push)
#pragma pack(1)
// 包含文件的版本信息。 此信息与语言和代码页无关
// https://learn.microsoft.com/zh-cn/windows/win32/menurc/vs-versioninfo
typedef struct {
    WORD             wLength;       // VS_VERSIONINFO 结构的长度（以字节为单位）,此长度不包括在 32 位边界上对齐任何后续版本资源数据的填充
    WORD             wValueLength;  // Value 成员的长度（以字节为单位）
    WORD             wType;         // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR            szKey[15];     // Unicode 字符串 L“VS_VERSION_INFO”
    WORD             Padding1;      // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
    //VS_FIXEDFILEINFO Value
    //WORD             Padding2
    //WORD             Children
} VS_VERSIONINFO, * PVS_VERSIONINFO;

#pragma pack(pop)

CVersionNumber::CVersionNumber()
    :m_nVer{ 0 }
{
};

CVersionNumber::CVersionNumber(const _tstring& strVer)
    :
    m_nVer{ 0 }
{
    _stscanf_s(strVer.c_str(), _T("%hd.%hd.%hd.%hd"), &m_nVer[0], &m_nVer[1], &m_nVer[2], &m_nVer[3]);
}

CVersionNumber::CVersionNumber(WORD v1, WORD v2, WORD v3, WORD v4)
    :m_nVer{ v1, v2, v3, v4 }
{
}

CVersionNumber& CVersionNumber::operator = (const _tstring& strVer)
{
    _stscanf_s(strVer.c_str(), _T("%hd.%hd.%hd.%hd"), &m_nVer[0], &m_nVer[1], &m_nVer[2], &m_nVer[3]);
    return *this;
}

int CVersionNumber::_Compare(const CVersionNumber& ref) const
{
    for (int i = 0; i < _countof(m_nVer); i++)
    {
        if (m_nVer[i] != ref.m_nVer[i])
        {
            return (m_nVer[i] > ref.m_nVer[i] ? 1 : -1);
        }
    }

    return 0;
}

_tstring CVersionNumber::GetString() const
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
        m_nVer[0],
        m_nVer[1],
        m_nVer[2],
        m_nVer[3]
    );

    return szBuf;
}

bool CVersionNumber::IsEmpty() const
{
    for (const auto& item : m_nVer)
    {
        if (0 != item)
        {
            return false;
        }
    }

    return true;
}

void CVersionNumber::Clear()
{
    for (auto& item : m_nVer)
    {
        item = 0;
    }
}

bool CVersionNumber::operator == (const CVersionNumber& ref)
{
    return _Compare(ref) == 0;
}

bool CVersionNumber::operator != (const CVersionNumber& ref)
{
    return _Compare(ref) != 0;
}

bool CVersionNumber::operator < (const CVersionNumber& ref)
{
    return _Compare(ref) < 0;
}

bool CVersionNumber::operator <= (const CVersionNumber& ref)
{
    return _Compare(ref) <= 0;
}

bool CVersionNumber::operator > (const CVersionNumber& ref)
{
    return _Compare(ref) > 0;
}

bool CVersionNumber::operator >= (const CVersionNumber& ref)
{
    return _Compare(ref) >= 0;
}

CFileVersion::CFileVersion()
    :
    m_lpVerData(NULL)
{
    memset(&m_VsFixedFileInfo, 0, sizeof(m_VsFixedFileInfo));
}

CFileVersion::~CFileVersion()
{
    this->Close();
}

FILE_VERSION_INFO CFileVersion::GetVersionInfo(const _tstring& strFile)
{
    CFileVersion verInfo;
    verInfo.LoadFile(strFile);
    return verInfo.GetVersionInfo();
}

FILE_VERSION_INFO CFileVersion::GetCurrentModuleVersionInfo()
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szBuf, _countof(szBuf));
    return GetVersionInfo(szBuf);
}

bool CFileVersion::LoadFile(const _tstring& strFile)
{
    PVOID pFsRedirectionOldValue = NULL;
    bool isDisableWow64Fs = false;
    bool isSuccess = false;
    UINT cbTranslate = 0;
    DWORD dwVerSize;

    if (strFile.empty())
    {
        return false;
    }

    this->Close();

    m_VersionInfo = FILE_VERSION_INFO();

    // 禁用文件重定向
    isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&pFsRedirectionOldValue);

    do
    {
        // 获取版本信息数据大小
        dwVerSize = ::GetFileVersionInfoSize(strFile.c_str(), 0);
        if (0 == dwVerSize)
        {
            break;
        }

        // 分配版本信息缓冲
        m_lpVerData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwVerSize);
        if (!m_lpVerData)
        {
            break;
        }

        // 获取版本信息
        if (!::GetFileVersionInfo(strFile.c_str(), 0, dwVerSize, m_lpVerData))
        {
            break;
        }

        // 获取语言代码
        LANGANDCODEPAGE* lpTranslate = NULL;
        if (!::VerQueryValue(m_lpVerData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate))
        {
            break;
        }

        m_Translate = *lpTranslate;

        // 获取 VS_FIXEDFILEINFO 信息
        PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)m_lpVerData;
        if (0 != lpVersion->wValueLength)
        {
            VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));

            DWORD dwAlign = 4;
            DWORD_PTR dwPadding = ((DWORD_PTR)pFixedFileInfo % dwAlign);
            if (0 != dwPadding)
            {
                pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)pFixedFileInfo + (dwAlign - dwPadding));
            }

            m_VsFixedFileInfo = *pFixedFileInfo;
        }

        // 加载所有信息
        _LoadAllInfo();

        isSuccess = true;

    } while (false);

    // 不成功则关闭
    if (!isSuccess)
    {
        this->Close();
    }

    // 恢复文件重定向
    if (isDisableWow64Fs)
    {
        ::Wow64RevertWow64FsRedirection(pFsRedirectionOldValue);
    }

    return isSuccess;
}

void CFileVersion::Close()
{
    if (m_lpVerData)
    {
        HeapFree(::GetProcessHeap(), 0, m_lpVerData);
        m_lpVerData = NULL;
    }
}

void CFileVersion::_LoadAllInfo()
{
    m_VersionInfo.strComments = _QueryInfo(_T("Comments"));
    m_VersionInfo.strInternalName = _QueryInfo(_T("InternalName"));
    m_VersionInfo.strProductName = _QueryInfo(_T("ProductName"));
    m_VersionInfo.strCompanyName = _QueryInfo(_T("CompanyName"));
    m_VersionInfo.strLegalCopyright = _QueryInfo(_T("LegalCopyright"));
    m_VersionInfo.strProductVersion = _QueryInfo(_T("ProductVersion"));
    m_VersionInfo.strFileDescription = _QueryInfo(_T("FileDescription"));
    m_VersionInfo.strLegalTrademarks = _QueryInfo(_T("LegalTrademarks"));
    m_VersionInfo.strPrivateBuild = _QueryInfo(_T("PrivateBuild"));
    m_VersionInfo.strFileVersion = _QueryInfo(_T("FileVersion"));
    m_VersionInfo.strOriginalFilename = _QueryInfo(_T("OriginalFilename"));
    m_VersionInfo.strSpecialBuild = _QueryInfo(_T("SpecialBuild"));
    m_VersionInfo.strFileVersionEx = GetFileVersionEx();
    m_VersionInfo.strProductVersionEx = GetProductVersionEx();

    m_VersionInfo.FileVerNumber = m_VersionInfo.strFileVersionEx.c_str();
    m_VersionInfo.ProductVerNumber = m_VersionInfo.strProductVersionEx.c_str();

}

_tstring CFileVersion::_QueryInfo(const _tstring& strName) const
{
    _tstring strRes;
    TCHAR strQuery[MAX_PATH] = { 0 };
    LPCTSTR lpQueryRes = NULL;
    UINT uQueryCchSize;

    do
    {
        if (!m_lpVerData)
        {
            break;
        }

        (void)::StringCchPrintf(strQuery, _countof(strQuery),
            _T("\\StringFileInfo\\%04x%04x\\%s"),
            m_Translate.wLanguage,
            m_Translate.wCodePage,
            strName.c_str());

        if (!::VerQueryValue(m_lpVerData, strQuery, (LPVOID*)&lpQueryRes, &uQueryCchSize))
        {
            break;
        }

        strRes = lpQueryRes;

    } while (false);

    return strRes;
}

_tstring CFileVersion::GetComments() const
{
    return m_VersionInfo.strComments;
}

_tstring CFileVersion::GetInternalName() const
{
    return m_VersionInfo.strInternalName;
}

_tstring CFileVersion::GetProductName() const
{
    return m_VersionInfo.strProductName;
}

_tstring CFileVersion::GetCompanyName() const
{
    return m_VersionInfo.strCompanyName;
}

_tstring CFileVersion::GetLegalCopyright() const
{
    return m_VersionInfo.strLegalCopyright;
}

_tstring CFileVersion::GetProductVersion() const
{
    return m_VersionInfo.strProductVersion;
}

_tstring CFileVersion::GetFileDescription() const
{
    return m_VersionInfo.strFileDescription;
}

_tstring CFileVersion::GetLegalTrademarks() const
{
    return m_VersionInfo.strLegalTrademarks;
}

_tstring CFileVersion::GetPrivateBuild() const
{
    return m_VersionInfo.strPrivateBuild;
}

_tstring CFileVersion::GetFileVersion() const
{
    return m_VersionInfo.strFileVersion;
}

_tstring CFileVersion::GetOriginalFilename() const
{
    return m_VersionInfo.strOriginalFilename;
}

_tstring CFileVersion::GetSpecialBuild() const
{
    return m_VersionInfo.strSpecialBuild;
}

_tstring CFileVersion::GetFileVersionEx() const
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
        HIWORD(m_VsFixedFileInfo.dwFileVersionMS),
        LOWORD(m_VsFixedFileInfo.dwFileVersionMS),
        HIWORD(m_VsFixedFileInfo.dwFileVersionLS),
        LOWORD(m_VsFixedFileInfo.dwFileVersionLS)
    );

    return szBuf;
}

_tstring CFileVersion::GetProductVersionEx() const
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
        HIWORD(m_VsFixedFileInfo.dwProductVersionMS),
        LOWORD(m_VsFixedFileInfo.dwProductVersionMS),
        HIWORD(m_VsFixedFileInfo.dwProductVersionLS),
        LOWORD(m_VsFixedFileInfo.dwProductVersionLS)
    );

    return szBuf;
}

VS_FIXEDFILEINFO CFileVersion::GetFixedFileInfo() const
{
    return m_VsFixedFileInfo;
}

FILE_VERSION_INFO CFileVersion::GetVersionInfo() const
{
    return m_VersionInfo;
}
