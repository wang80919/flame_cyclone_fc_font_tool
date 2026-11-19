#pragma once

#include <windows.h>
#include <string>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

// 版本号辅助类
class CVersionNumber
{
public:

    // 无参构造
    CVersionNumber();

    // 从版本字符串构造
    CVersionNumber(const _tstring& strVer);

    // 从指定版本号构造
    CVersionNumber(WORD v1, WORD v2, WORD v3, WORD v4);

    // 从版本字符串构造
    CVersionNumber& operator = (const _tstring& strVer);

    //获取版本号字符串
    _tstring GetString() const;

    // 是否为空
    bool IsEmpty() const;

    // 清空
    void Clear();

    // 比较运算重载
    bool operator == (const CVersionNumber& ref);
    bool operator != (const CVersionNumber& ref);
    bool operator < (const CVersionNumber& ref);
    bool operator <= (const CVersionNumber& ref);
    bool operator > (const CVersionNumber& ref);
    bool operator >= (const CVersionNumber& ref);

private:

    // 比较版本号大小
    int _Compare(const CVersionNumber& ref) const;

private:

    WORD m_nVer[4];         //版本号
};

typedef struct _FILE_VERSION_INFO
{
    _tstring strComments;                       //文件注释
    _tstring strInternalName;                   //内部名称
    _tstring strProductName;                    //产品名称
    _tstring strCompanyName;                    //公司名称
    _tstring strLegalCopyright;                 //法律版权
    _tstring strProductVersion;                 //产品版本
    _tstring strFileDescription;                //文件描述
    _tstring strLegalTrademarks;                //合法商标
    _tstring strPrivateBuild;                   //私有构建
    _tstring strFileVersion;                    //文件版本
    _tstring strOriginalFilename;               //原始文件名
    _tstring strSpecialBuild;                   //特殊构建
    _tstring strFileVersionEx;                  //文件版本(从 VS_FIXEDFILEINFO中 获取)
    _tstring strProductVersionEx;               //产品版本(从 VS_FIXEDFILEINFO中 获取)

    CVersionNumber FileVerNumber;               //文件版本号
    CVersionNumber ProductVerNumber;            //产品版本号
}FILE_VERSION_INFO;

class CFileVersion
{
    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;

        LANGANDCODEPAGE()
            :
            wLanguage(0),
            wCodePage(0)
        {

        }
    };

public:
    CFileVersion();
    ~CFileVersion();

    CFileVersion operator = (const CFileVersion& r) = delete;
    static FILE_VERSION_INFO GetVersionInfo(const _tstring& strFile);
    static FILE_VERSION_INFO GetCurrentModuleVersionInfo();

public:
    bool LoadFile(const _tstring& strFile);             //加载文件信息
    void Close();

    FILE_VERSION_INFO GetVersionInfo() const;           //整个版本信息

    _tstring GetComments() const;                       //文件注释
    _tstring GetInternalName() const;                   //内部名称
    _tstring GetProductName() const;                    //产品名称
    _tstring GetCompanyName() const;                    //公司名称
    _tstring GetLegalCopyright() const;                 //法律版权
    _tstring GetProductVersion() const;                 //产品版本
    _tstring GetFileDescription() const;                //文件描述
    _tstring GetLegalTrademarks() const;                //合法商标
    _tstring GetPrivateBuild() const;                   //私有版本
    _tstring GetFileVersion() const;                    //文件版本
    _tstring GetOriginalFilename() const;               //原始文件名
    _tstring GetSpecialBuild() const;                   //特别版本
    _tstring GetFileVersionEx() const;                  //文件版本(从 VS_FIXEDFILEINFO中 获取)
    _tstring GetProductVersionEx() const;               //产品版本(从 VS_FIXEDFILEINFO中 获取)
    VS_FIXEDFILEINFO GetFixedFileInfo() const;          //文件的版本信息

private:
    _tstring _QueryInfo(const _tstring& strName) const;
    void _LoadAllInfo();

    LPVOID m_lpVerData;                                 //文件信息数据指针
    LANGANDCODEPAGE m_Translate;                        //语言代码页
    VS_FIXEDFILEINFO m_VsFixedFileInfo;                 //文件的版本信息
    FILE_VERSION_INFO m_VersionInfo;                    //版本信息
};