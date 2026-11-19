#pragma once

#include <Windows.h>
#include <WinInet.h>
#include <string>
#include <vector>
#include <functional>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

typedef struct _DOWNLOAD_URL_COMPONENTS DOWNLOAD_URL_COMPONENTS, * LPDOWNLOAD_URL_COMPONENTS;
typedef struct _DOWNLOAD_PACKAGE_INFO DOWNLOAD_PACKAGE_INFO, * LPDOWNLOAD_PACKAGE_INFO;

#define WININET_HTTPS_DOWNLOAD_AGENT LR"(Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36 Edg/122.0.0.0)"

class CHttpsResponse
{
public:
    CHttpsResponse() :code(0) {}
    std::string result;         //响应结果
    DWORD code;                 //响应状态码
};

// https下载类
class CHttpsDownload
{
public:
    CHttpsDownload();
    ~CHttpsDownload();

    //
    // @brief: 获取下载内容大小
    // @param: strUrl           URL链接
    // @param: pUllLength       内容长度缓冲指针
    // @ret: bool               操作成功与否
    bool GetContentLength(
        const _tstring& strUrl, 
        PULONGLONG pUllLength
    );

    //
    // @brief: 下载保存到文件
    // @param: strUrl           URL链接
    // @param: strFile          保存文件路径
    // @param: cbProgress       下载进度回调函数
    // @param: dwThreadCount    下载线程数
    // @ret: bool               操作成功与否
    bool DownloadToFile(
        const _tstring& strUrl, 
        const _tstring& strFile, 
        std::function<void(double lfProgress, ULONGLONG ullCur, ULONGLONG ullTotal, ULONGLONG ullSpeed)> cbProgress = nullptr,
        DWORD dwThreadCount = 8
    );

    //
    // @brief: 下载保存到缓存
    // @param: strUrl           URL链接
    // @param: lpBuf            保存缓冲
    // @param: dwBufSize        缓冲大小
    // @param: cbProgress       下载进度回调函数
    // @param: dwThreadCount    下载线程数
    // @ret: bool               操作成功与否
    bool DownloadToBuffer(
        const _tstring& strUrl, 
        LPVOID lpBuf, 
        DWORD dwBufSize, 
        std::function<void(double lfProgress, ULONGLONG ullCur, ULONGLONG ullTotal, ULONGLONG ullSpeed)> cbProgress = nullptr,
        DWORD dwThreadCount = 8
    );

    //
    // @brief: 发送Get请求
    // @param: strUrl           URL链接
    // @param: strHeader        请求头
    // @ret: CHttpsResponse     响应结果
    CHttpsResponse Get(const _tstring& strUrl,
        _tstring strHeader = _T("Accept: */*\r\nContent-Type:application/json;charset=UTF-8")
    );

    //
    // @brief: 发送Post请求
    // @param: strUrl           URL链接
    // @param: strParam         请求参数
    // @param: strHeader        请求头
    // @ret: CHttpsResponse     响应结果
    CHttpsResponse Post(const _tstring& strUrl,
        std::string& strParam,
        _tstring strHeader = _T("Accept: */*\r\nContent-Type:application/json;charset=UTF-8")
    );

private:

    // 清空文件内容
    void _TruncateFile(const _tstring& strPath);

    // 下载前备份文件
    bool _BackupFile(const _tstring& strPath);

    // 分段下载到文件
    bool _PartialDownload(
        const _tstring& strUrl, 
        const _tstring& strUrlFileName, 
        ULONGLONG ullContentLength, 
        DWORD dwThreadCount
    );
    
    // 分段下载到缓冲
    bool _PartialDownload(
        const _tstring& strUrl, 
        LPVOID lpBuf,
        DWORD dwBufSize, 
        ULONGLONG ullContentLength, 
        DWORD dwThreadCount
    );

    // 分解链接
    bool _CrackUrl(
        const _tstring& strUrl, 
        LPDOWNLOAD_URL_COMPONENTS lpUci
    );

    // 查询是否支持接收范围
    bool _IsSupportAcceptRanges(HINTERNET hRequest);

    // 查询资源大小
    bool _QueryContentLength(
        HINTERNET hRequest, 
        PULONGLONG lpUllContentLength
    );

    // 获取状态码
    long _GetStatusCodes(HINTERNET hRequest);

    // 发送请求
    bool _SendRequest(
        HINTERNET hRequest, 
        LPCTSTR lpszHeaders,
        DWORD dwHeadersLength, 
        LPVOID lpData, 
        DWORD dwSize
    );

    // 读取网络流到文件
    bool _InternetReadToFile(
        HINTERNET hRequest, 
        LPDOWNLOAD_PACKAGE_INFO lpDownloadPackage
    );

    // 读取网络流到缓冲
    bool _InternetReadToBuffer(
        HINTERNET hRequest, 
        LPDOWNLOAD_PACKAGE_INFO lpDownloadPackage
    );

    // 读取网络流到响应数据
    bool _InternetReadToResponse(
        HINTERNET hRequest, 
        std::string& strResponse
    );

    // 设置请求数据范围
    bool _SetRequestDataRange(
        HINTERNET hRequest, 
        LONGLONG nBegin, 
        LONGLONG nEng, 
        bool isHasEnd = TRUE
    );

    // 下载分包
    bool _DownloadPackage(LPDOWNLOAD_PACKAGE_INFO pPackageInfo);
    
    void _SleepMillisecond(int millisecond) const;

    // 打印警告
    void _PrintWarn(LPCTSTR lpszError) const;

    // 打印错误
    void _PrintError(LPCTSTR lpszError) const;

    // 异步回调函数
    static void _InternetStatusCallback(
        HINTERNET hInternet, 
        DWORD_PTR dwContext, 
        DWORD dwInternetStatus, 
        LPVOID lpvStatusInformation, 
        DWORD dwStatusInformationLength
    );

private:

    std::function<void(double lfProgress, ULONGLONG ullCur, ULONGLONG ullTotal, ULONGLONG ullSpeed)> m_cbProgress;
    bool m_bAbort;
    bool m_bSupportAcceptRanges;
    bool m_bHasSize;
};