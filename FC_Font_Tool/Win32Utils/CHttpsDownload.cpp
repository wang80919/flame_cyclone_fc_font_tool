#include "CHttpsDownload.h"
#include <strsafe.h>
#include <time.h>
#include <thread>
#include <algorithm>

#pragma comment(lib, "WinInet.lib")

#define INTERNET_MAX_CONNS_PER_SERVER           (64)        //服务器最大连接数
#define DOWNLOAD_BLOCK_SIZE                     (1024 * 8)  //下载缓冲大小
#define DOWNLOAD_MINIMUM_SIZE                   (1024 * 4)  //单线程最小下载大小
#define PROGRESS_INTERVAL_TIME                  (100)      //下载进度更新时间
#define BACKUP_MAX_CONUT                        (0)     // 最多备份数量

const DWORD HTTP_REQUEST_FLAGS  =   INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |     // 禁用此特殊类型的重定向检测。 使用此标志时，WinINet 函数以透明方式允许从 HTTPS 重定向到 HTTP URL
                                    INTERNET_FLAG_KEEP_CONNECTION |             // 对连接使用保持活动语义（如果可用）
                                    INTERNET_FLAG_NO_AUTH |                     // 不会自动尝试身份验证
                                    INTERNET_FLAG_NO_COOKIES |                  // 不会自动向请求添加 Cookie 标头，也不会自动将返回的 Cookie 添加到 Cookie 数据库
                                    INTERNET_FLAG_NO_UI |                       // 禁用 Cookie 对话框
                                    INTERNET_FLAG_RELOAD;                       // 强制从源服务器下载请求的文件、对象或目录列表，而不是从缓存下载

const DWORD HTTPS_REQUEST_FLAGS =   HTTP_REQUEST_FLAGS |

                                    // HTTPS 设置
                                    INTERNET_FLAG_SECURE |                      // 使用安全事务语义。 这转换为使用安全套接字层/专用通信技术 (SSL/PCT) ，并且仅在 HTTP 请求中有意义
                                    INTERNET_FLAG_IGNORE_CERT_CN_INVALID |      // 忽略X509 证书中的错误通用名称
                                    SECURITY_FLAG_IGNORE_UNKNOWN_CA |           // 忽略未知证书颁发机构问题
                                    INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;     // 忽略过期的 X509 证书


// 下载分包信息
typedef struct _DOWNLOAD_PACKAGE_INFO
{
    enum DownloadStatus {
        DS_Downloading = 0,
        DS_DownloadFinish = 1,
        DS_DownloadFailed = 2,
        DS_DownloadCancel = 3,
    };

    _tstring strUrl;
    _tstring strFileName;
    LPVOID lpBuffer;
    ULONGLONG ullBeginPos;
    ULONGLONG ullContentLength;
    ULONGLONG ullDownloaded;
    DWORD dwStatus;
    DWORD dwPartID;

    _DOWNLOAD_PACKAGE_INFO(const _tstring& strUrl, const _tstring& strFileName, ULONGLONG ullBeginPos, ULONGLONG ullContentLength, DWORD dwPartID)
    {
        this->strUrl = strUrl;
        this->strFileName = strFileName;
        this->lpBuffer = NULL;
        this->ullBeginPos = ullBeginPos;
        this->ullContentLength = ullContentLength;
        this->ullDownloaded = 0;
        this->dwPartID = dwPartID;
        this->dwStatus = DownloadStatus::DS_Downloading;
    }

    _DOWNLOAD_PACKAGE_INFO(const _tstring& strUrl, LPVOID lpBuffer, ULONGLONG ullBeginPos, ULONGLONG ullContentLength, DWORD dwPartID)
    {
        this->strUrl = strUrl;
        this->lpBuffer = lpBuffer;
        this->ullBeginPos = ullBeginPos;
        this->ullContentLength = ullContentLength;
        this->ullDownloaded = 0;
        this->dwPartID = dwPartID;
        this->dwStatus = DownloadStatus::DS_Downloading;
    }

}DOWNLOAD_PACKAGE_INFO, *LPDOWNLOAD_PACKAGE_INFO;

typedef struct _DOWNLOAD_URL_COMPONENTS
{
    TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
    TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    TCHAR szUrlPath[INTERNET_MAX_URL_LENGTH];
    TCHAR szExtraInfo[MAX_PATH];
    URL_COMPONENTS uc = { 0 };

    _DOWNLOAD_URL_COMPONENTS()
    {
        ZeroMemory(this, sizeof(_DOWNLOAD_URL_COMPONENTS));
        this->uc.dwStructSize = sizeof(this->uc);
        this->uc.lpszUrlPath = this->szUrlPath;
        this->uc.dwUrlPathLength = _countof(this->szUrlPath);
        this->uc.lpszScheme = this->szScheme;
        this->uc.dwSchemeLength = _countof(this->szScheme);
        this->uc.lpszHostName = this->szHostName;
        this->uc.dwHostNameLength = _countof(this->szHostName);
        this->uc.lpszUserName = this->szUserName;
        this->uc.dwUserNameLength = _countof(this->szUserName);
        this->uc.lpszPassword = this->szPassword;
        this->uc.dwPasswordLength = _countof(this->szPassword);
        this->uc.lpszExtraInfo = this->szExtraInfo;
        this->uc.dwExtraInfoLength = _countof(this->szExtraInfo);
    }
}DOWNLOAD_URL_COMPONENTS;

CHttpsDownload::CHttpsDownload()
    :
    m_bAbort(false),
    m_bSupportAcceptRanges(false),
    m_bHasSize(false)
{
}

CHttpsDownload::~CHttpsDownload()
{

}

bool CHttpsDownload::GetContentLength(
    const _tstring& strUrl, 
    PULONGLONG lpUllContentLength
)
{
    DOWNLOAD_URL_COMPONENTS ci;
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool bSuccess = false;

    if (strUrl.empty() || NULL == lpUllContentLength)
    {
        return false;
    }

    // 分解URL
    if (!_CrackUrl(strUrl, &ci))
    {
        return false;
    }

    // 协议检查
    if (!(INTERNET_SCHEME_HTTPS == ci.uc.nScheme || INTERNET_SCHEME_HTTP == ci.uc.nScheme))
    {
        _PrintError(_T("Scheme"));
        return false;
    }

    do
    {
        // 初始化应用程序对 WinINet 函数的使用
        hInternet = ::InternetOpen(WININET_HTTPS_DOWNLOAD_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (NULL == hInternet)
        {
            _PrintError(_T("InternetOpen"));
            break;
        }

        // 打开给定站点的 HTTP 会话
        hConnect = ::InternetConnect(hInternet, ci.szHostName, ci.uc.nPort, ci.szUserName, ci.szPassword, INTERNET_SERVICE_HTTP, 0, NULL);
        if (NULL == hConnect)
        {
            _PrintError(_T("InternetConnect"));
            break;
        }

        // 修改每个服务器的连接数
        DWORD dwMaxConn = INTERNET_MAX_CONNS_PER_SERVER;
        if (!::InternetSetOption(hConnect, INTERNET_OPTION_MAX_CONNS_PER_SERVER, (LPVOID)&dwMaxConn, sizeof(dwMaxConn)))
        {
            _PrintWarn(_T("InternetSetOption"));
        }

        if (!::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, (LPVOID)&dwMaxConn, sizeof(dwMaxConn)))
        {
            _PrintWarn(_T("InternetSetOption"));
        }

        // 创建 HTTPS / HTTP 请求句柄
        DWORD dwFlags = (INTERNET_SCHEME_HTTPS == ci.uc.nScheme) ? HTTPS_REQUEST_FLAGS : HTTP_REQUEST_FLAGS;
        hRequest = ::HttpOpenRequest(hConnect, _T("GET"), ci.szUrlPath, NULL, NULL, NULL, dwFlags, 0);
        if (NULL == hRequest)
        {
            _PrintError(_T("HttpOpenRequest"));
            break;
        }

        // 发送请求
        if (!_SendRequest(hRequest, nullptr, 0, nullptr, 0))
        {
            _PrintError(_T("SendRequest"));
            break;
        }

        DWORD statusCodes = _GetStatusCodes(hRequest);
        if (statusCodes < 200 || statusCodes >= 300)
        {
            char szResponseBuffer[MAX_PATH] = { 0 };
            DWORD dwBufferLength = sizeof(szResponseBuffer);
            ::HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_TEXT, szResponseBuffer, &dwBufferLength, NULL);

            char szErrorBuf[MAX_PATH] = { 0 };
            StringCchPrintfA(szErrorBuf, _countof(szErrorBuf),
                R"({"status":%d,"data":null,"message":"%d %s"})",
                statusCodes, statusCodes, szResponseBuffer
            );

            printf("%s\r\n", szErrorBuf);

            break;
        }

        // 记录是否支持接收范围请求
        m_bSupportAcceptRanges = _IsSupportAcceptRanges(hRequest);

        // 查询文件大小
        if (!_QueryContentLength(hRequest, lpUllContentLength))
        {
            _PrintError(_T("QueryContentLength"));
            break;
        }

        bSuccess = true;

    } while (false);

    // 释放资源
    if (hRequest)
    {
        ::InternetCloseHandle(hRequest);
    }

    if (hConnect)
    {
        ::InternetCloseHandle(hConnect);
    }

    if (hInternet)
    {
        ::InternetCloseHandle(hInternet);
    }

    return bSuccess;
}

bool CHttpsDownload::DownloadToFile(
    const _tstring& strUrl, 
    const _tstring& strFile, 
    std::function<void(double lfProgress, ULONGLONG ullCur, ULONGLONG ullTotal, ULONGLONG ullSpeed)> cbProgress,
    DWORD dwThreadCount
)
{
    ULONGLONG ullContentLength = 0;
    _tstring strTmpFile = strFile + _T(".tmp");

    if (strUrl.empty() || strFile.empty())
    {
        return false;
    }

    m_cbProgress = cbProgress;

    //限制线程数量
    dwThreadCount = dwThreadCount < 1 ? 1 : dwThreadCount;
    dwThreadCount = dwThreadCount > 64 ? 64 : dwThreadCount;

    // 获取资源大小
    if (!GetContentLength(strUrl, &ullContentLength))
    {
        _PrintError(_T("GetContentLength"));
        return false;
    }

    // 不支持范围请求则仅单线程下载
    if (!m_bSupportAcceptRanges || !m_bHasSize)
    {
        dwThreadCount = 1;
    }

    // 备份文件
    if (!_BackupFile(strFile))
    {
        _PrintError(_T("BackupFile"));
        return false;
    }

    // 删除文件, 防止下载后改名冲突
    if (!::DeleteFile(strFile.c_str()))
    {
        if (ERROR_FILE_NOT_FOUND != ::GetLastError())
        {
            return false;
        }
    }

    // 清空一下临时文件(如果存在的话)
    (void)_TruncateFile(strTmpFile);

    // 多线程分段下载
    if (!_PartialDownload(strUrl, strTmpFile, ullContentLength, dwThreadCount))
    {
        _PrintError(_T("PartialDownload"));
        return false;
    }

    // 下载完成则修改文件名
    ::MoveFile(strTmpFile.c_str(), strFile.c_str());

    return true;
}

bool CHttpsDownload::DownloadToBuffer(
    const _tstring& strUrl, 
    LPVOID lpBuf, 
    DWORD dwBufSize, 
    std::function<void(double lfProgress, ULONGLONG ullCur, ULONGLONG ullTotal, ULONGLONG ullSpeed)> cbProgress,
    DWORD dwThreadCount
)
{
    ULONGLONG ullContentLength = 0;

    if (strUrl.empty() || NULL == lpBuf || 0 == dwBufSize)
    {
        return false;
    }

    m_cbProgress = cbProgress;

    //限制线程数量
    dwThreadCount = dwThreadCount < 2 ? 2 : dwThreadCount;
    dwThreadCount = dwThreadCount > 64 ? 64 : dwThreadCount;

    // 获取资源大小
    if (!GetContentLength(strUrl, &ullContentLength))
    {
        _PrintError(_T("GetContentLength"));
        return false;
    }

    // 不支持范围请求则仅单线程下载
    if (!m_bSupportAcceptRanges || !m_bHasSize)
    {
        dwThreadCount = 1;
    }

    // 多线程分段下载
    if (!_PartialDownload(strUrl, lpBuf, dwBufSize, ullContentLength, dwThreadCount))
    {
        _PrintError(_T("PartialDownload"));
        return false;
    }

    return true;
}

CHttpsResponse CHttpsDownload::Get(
    const _tstring& strUrl, 
    _tstring strHeader/* = _T("Content-Type:application/json;charset=UTF-8")*/
)
{
    CHttpsResponse httpsResponse;

    DOWNLOAD_URL_COMPONENTS ci;
    ULONGLONG ullContentLength = 0;
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool bSuccess = false;

    // 分解URL
    if (!_CrackUrl(strUrl, &ci))
    {
        return httpsResponse;
    }

    do
    {
        // 初始化应用程序对 WinINet 函数的使用
        hInternet = ::InternetOpen(WININET_HTTPS_DOWNLOAD_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (NULL == hInternet)
        {
            _PrintError(_T("InternetOpen"));
            break;
        }

        // 打开给定站点的 HTTP 会话
        hConnect = ::InternetConnect(hInternet, ci.szHostName, ci.uc.nPort, ci.szUserName, ci.szPassword, INTERNET_SERVICE_HTTP, 0, NULL);
        if (NULL == hConnect)
        {
            _PrintError(_T("InternetConnect"));
            break;
        }

        // 创建 HTTPS / HTTP 请求句柄
        DWORD dwFlags = (INTERNET_SCHEME_HTTPS == ci.uc.nScheme) ? HTTPS_REQUEST_FLAGS : HTTP_REQUEST_FLAGS;
        hRequest = ::HttpOpenRequest(hConnect, _T("GET"), ci.szUrlPath, NULL, NULL, NULL, dwFlags, 0);
        if (NULL == hRequest)
        {
            _PrintError(_T("HttpOpenRequest"));
            break;
        }

        // 发送请求
        if (!_SendRequest(hRequest, strHeader.data(), (DWORD)strHeader.size(), nullptr, 0))
        {
            _PrintError(_T("SendRequest"));
            break;
        }

        // 记录状态码
        httpsResponse.code = _GetStatusCodes(hRequest);

        // 查询文件大小
        if (!_QueryContentLength(hRequest, &ullContentLength))
        {
            _PrintError(_T("QueryContentLength"));
            break;
        }

        if (!_InternetReadToResponse(hRequest, httpsResponse.result))
        {
            _PrintError(_T("SaveToBuffer"));
            break;
        }

        bSuccess = true;
    } while (false);

    // 释放资源
    if (hRequest)
    {
        ::InternetCloseHandle(hRequest);
    }

    if (hConnect)
    {
        ::InternetCloseHandle(hConnect);
    }

    if (hInternet)
    {
        ::InternetCloseHandle(hInternet);
    }

    return httpsResponse;
}

CHttpsResponse CHttpsDownload::Post(
    const _tstring& strUrl, 
    std::string& strParam, 
    _tstring strHeader
)
{
    CHttpsResponse httpsResponse;

    DOWNLOAD_URL_COMPONENTS ci;
    ULONGLONG ullContentLength = 0;
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool bSuccess = false;

    // 分解URL
    if (!_CrackUrl(strUrl, &ci))
    {
        return httpsResponse;
    }

    do
    {
        // 初始化应用程序对 WinINet 函数的使用
        hInternet = ::InternetOpen(WININET_HTTPS_DOWNLOAD_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (NULL == hInternet)
        {
            _PrintError(_T("InternetOpen"));
            break;
        }

        // 打开给定站点的 HTTP 会话
        hConnect = ::InternetConnect(hInternet, ci.szHostName, ci.uc.nPort, ci.szUserName, ci.szPassword, INTERNET_SERVICE_HTTP, 0, NULL);
        if (NULL == hConnect)
        {
            _PrintError(_T("InternetConnect"));
            break;
        }

        // 创建 HTTPS / HTTP 请求句柄
        DWORD dwFlags = (INTERNET_SCHEME_HTTPS == ci.uc.nScheme) ? HTTPS_REQUEST_FLAGS : HTTP_REQUEST_FLAGS;
        hRequest = ::HttpOpenRequest(hConnect, _T("POST"), ci.szUrlPath, NULL, NULL, NULL, dwFlags, 0);
        if (NULL == hRequest)
        {
            _PrintError(_T("HttpOpenRequest"));
            break;
        }

        // 发送请求
        if (!_SendRequest(hRequest, strHeader.c_str(), (DWORD)strHeader.size(), (LPVOID)strParam.data(), (DWORD)strParam.size()))
        {
            _PrintError(_T("SendRequest"));
            break;
        }

        // 记录状态码
        httpsResponse.code = _GetStatusCodes(hRequest);

        // 查询文件大小
        if (!_QueryContentLength(hRequest, &ullContentLength))
        {
            _PrintError(_T("QueryContentLength"));
            break;
        }

        if (!_InternetReadToResponse(hRequest, httpsResponse.result))
        {
            _PrintError(_T("SaveToBuffer"));
            break;
        }

        bSuccess = true;
    } while (false);

    // 释放资源
    if (hRequest)
    {
        ::InternetCloseHandle(hRequest);
    }

    if (hConnect)
    {
        ::InternetCloseHandle(hConnect);
    }

    if (hInternet)
    {
        ::InternetCloseHandle(hInternet);
    }

    return httpsResponse;
}

void CHttpsDownload::_TruncateFile(const _tstring& strPath)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    hFile = ::CreateFile(strPath.c_str(),
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE,
        NULL,
        TRUNCATE_EXISTING,
        FILE_ATTRIBUTE_ARCHIVE,
        NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }
}

bool CHttpsDownload::_BackupFile(const _tstring& strPath)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    _tstring strBakFile;
    bool isBackupOk = false;

    if (0 == BACKUP_MAX_CONUT)
    {
        return true;
    }

    for (int i = 1; i <= BACKUP_MAX_CONUT; i++)
    {
        StringCchPrintf(szBuf, _countof(szBuf), _T("%02d"), i);
        strBakFile = strPath + _T(".") + szBuf + _T(".bak");
        if (::MoveFile(strPath.c_str(), strBakFile.c_str()))
        {
            isBackupOk = true;
            break;
        }

        if (ERROR_FILE_NOT_FOUND == ::GetLastError())
        {
            isBackupOk = true;
            break;
        }
    }

    //备份失败, 则删除最早一个备份文件
    if (!isBackupOk)
    {
        _tstring strOld;
        _tstring strNew;

        StringCchPrintf(szBuf, _countof(szBuf), _T("%02d"), 1);
        strOld = strPath + _T(".") + szBuf + _T(".bak");

        // 无法删除
        if (!::DeleteFile(strOld.c_str()))
        {
            return false;
        }

        for (int i = 1; i < BACKUP_MAX_CONUT; i++)
        {
            StringCchPrintf(szBuf, _countof(szBuf), _T("%02d"), i);
            strNew = strPath + _T(".") + szBuf + _T(".bak");;
            StringCchPrintf(szBuf, _countof(szBuf), _T("%02d"), i + 1);
            strOld = strPath + _T(".") + szBuf + _T(".bak");;
            if (!::MoveFile(strOld.c_str(), strNew.c_str()))
            {
                break;
            }
        }

        StringCchPrintf(szBuf, _countof(szBuf), _T("%02d"), BACKUP_MAX_CONUT);
        strBakFile = strPath + _T(".") + szBuf + _T(".bak");
        if (::MoveFile(strPath.c_str(), strBakFile.c_str()))
        {
            isBackupOk = true;
        }
    }

    return isBackupOk;
}

bool CHttpsDownload::_PartialDownload(
    const _tstring& strUrl, 
    const _tstring& strUrlFileName, 
    ULONGLONG ullContentLength, 
    DWORD dwThreadCount
)
{
    //资源过小检查
    if (ullContentLength < DOWNLOAD_MINIMUM_SIZE)
    {
        dwThreadCount = 1;
    }

    // 每个包大小
    ULONGLONG ullSinglePackageSize = ullContentLength / dwThreadCount;
    std::vector<std::thread> downloadThreads;

    // 保留容量, 防止发生空间分配导致迭代器失效
    std::vector<DOWNLOAD_PACKAGE_INFO> m_downloadPackages;
    m_downloadPackages.reserve(dwThreadCount);

    // 添加下载任务
    for (DWORD i = 0; i < dwThreadCount; i++)
    {
        ULONGLONG ullBeginPos = ullSinglePackageSize * i;
        ULONGLONG ullPackageSize = (i != dwThreadCount - 1) ? ullSinglePackageSize : ullContentLength - ullBeginPos;
        m_downloadPackages.emplace_back(DOWNLOAD_PACKAGE_INFO(strUrl, strUrlFileName, ullBeginPos, ullPackageSize, i));
        downloadThreads.emplace_back(std::thread(
            [this, &m_downloadPackages, i]() {
                if (!_DownloadPackage(&m_downloadPackages[i]))
                {
                    m_downloadPackages[i].dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFailed;
                }
            }));
    }

    // 创建进度报告线程
    {
        ULONGLONG ullLastDownload = 0;
        DWORD dwDownloadingCount = dwThreadCount;
        DWORD dwRefreshInterval = PROGRESS_INTERVAL_TIME;

        // 等待所有线程结束下载
        while (dwDownloadingCount)
        {
            ULONGLONG ullDownloaded = 0;
            ULONGLONG ullTotalLength = 0;
            dwDownloadingCount = 0;

            // 统计已下载量 与 总下载量
            for (const auto& item : m_downloadPackages)
            {
                ullDownloaded += item.ullDownloaded;
                ullTotalLength += item.ullContentLength;

                if (DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_Downloading == item.dwStatus)
                {
                    dwDownloadingCount++;
                }
            }

            // 进度回调
            if (m_cbProgress)
            {
                double lfProgress = (double)ullDownloaded / (double)ullTotalLength;
                m_cbProgress(lfProgress, ullDownloaded, ullTotalLength, (ullDownloaded - ullLastDownload) * 1000 / dwRefreshInterval);
            }

            ullLastDownload = ullDownloaded;
            _SleepMillisecond(dwRefreshInterval);
        }
    }

    // 等待所有下载线程结束
    for (auto &item : downloadThreads)
    {
        item.join();
    }

    // 获取每个线程的下载状态, 全部下载完成才算完成
    bool isAllFinish = std::all_of(m_downloadPackages.begin(), m_downloadPackages.end(), 
        [](const DOWNLOAD_PACKAGE_INFO& item) {
            return DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFinish == item.dwStatus;
        });

    return isAllFinish;
}

bool CHttpsDownload::_PartialDownload(
    const _tstring& strUrl, 
    LPVOID lpBuf, 
    DWORD dwBufSize, 
    ULONGLONG ullContentLength, 
    DWORD dwThreadCount
)
{
    //资源过小检查
    if (ullContentLength < DOWNLOAD_MINIMUM_SIZE || dwBufSize < DOWNLOAD_MINIMUM_SIZE)
    {
        dwThreadCount = 1;
    }

    // 每个包大小
    ullContentLength = ullContentLength < dwBufSize ? ullContentLength : dwBufSize;
    ULONGLONG ullSinglePackageSize = ullContentLength / dwThreadCount;
    std::vector<std::thread> downloadThreads;

    // 保留容量, 防止发生空间分配导致迭代器失效
    std::vector<DOWNLOAD_PACKAGE_INFO> m_downloadPackages;
    m_downloadPackages.reserve(dwThreadCount);

    // 添加下载任务
    for (DWORD i = 0; i < dwThreadCount; i++)
    {
        ULONGLONG ullBeginPos = ullSinglePackageSize * i;
        ULONGLONG ullPackageSize = (i != dwThreadCount - 1) ? ullSinglePackageSize : ullContentLength - ullBeginPos;
        m_downloadPackages.emplace_back(DOWNLOAD_PACKAGE_INFO(strUrl, (LPBYTE)lpBuf + ullBeginPos, ullBeginPos, ullPackageSize, i));
        downloadThreads.emplace_back(std::thread(
            [this, &m_downloadPackages, i]() {
                if (!_DownloadPackage(&m_downloadPackages[i]))
                {
                    m_downloadPackages[i].dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFailed;
                }
            }));
    }

    // 创建进度报告线程
    {
        ULONGLONG ullLastDownload = 0;
        DWORD dwDownloadingCount = dwThreadCount;
        DWORD dwRefreshInterval = PROGRESS_INTERVAL_TIME;

        // 等待所有线程结束下载
        while (dwDownloadingCount)
        {
            ULONGLONG ullDownloaded = 0;
            ULONGLONG ullTotalLength = 0;
            dwDownloadingCount = 0;

            // 统计已下载量 与 总下载量
            for (const auto& item : m_downloadPackages)
            {
                ullDownloaded += item.ullDownloaded;
                ullTotalLength += item.ullContentLength;

                if (DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_Downloading == item.dwStatus)
                {
                    dwDownloadingCount++;
                }
            }

            // 进度回调
            if (m_cbProgress)
            {
                double lfProgress = (double)ullDownloaded / (double)ullTotalLength;
                m_cbProgress(lfProgress, ullDownloaded, ullTotalLength, (ullDownloaded - ullLastDownload) * 1000 / dwRefreshInterval);
            }

            ullLastDownload = ullDownloaded;
            _SleepMillisecond(dwRefreshInterval);
        }
    }

    // 等待所有下载线程结束
    for (auto &item : downloadThreads)
    {
        item.join();
    }

    // 获取每个线程的下载状态, 全部下载完成才算完成
    bool isAllFinish = std::all_of(m_downloadPackages.begin(), m_downloadPackages.end(),
        [](const DOWNLOAD_PACKAGE_INFO& item) {
            return DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFinish == item.dwStatus;
        });

    return isAllFinish;
}

void CHttpsDownload::_SleepMillisecond(int millisecond) const
{
    int span = 10;

    if (millisecond < span)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(span));
        return;
    }

    // 分段休眠, 防止中断休眠不及时
    clock_t tBegin = clock();
    while (!m_bAbort)
    {
        if (clock() - tBegin > millisecond)
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(span));
    }
}

bool CHttpsDownload::_DownloadPackage(LPDOWNLOAD_PACKAGE_INFO pPackageInfo)
{
    DOWNLOAD_URL_COMPONENTS ci;
    ULONGLONG ullContentLength = 0;
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool bSuccess = false;

    // 分解URL
    if (!_CrackUrl(pPackageInfo->strUrl, &ci))
    {
        return false;
    }

    do
    {
        // 初始化应用程序对 WinINet 函数的使用
        hInternet = ::InternetOpen(WININET_HTTPS_DOWNLOAD_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (NULL == hInternet)
        {
            _PrintError(_T("InternetOpen"));
            break;
        }
        
        // 打开给定站点的 HTTP 会话
        hConnect = ::InternetConnect(hInternet, ci.szHostName, ci.uc.nPort, ci.szUserName, ci.szPassword, INTERNET_SERVICE_HTTP, 0, NULL);
        if (NULL == hConnect)
        {
            _PrintError(_T("InternetConnect"));
            break;
        }
        
        // 创建 HTTPS / HTTP 请求句柄
        DWORD dwFlags = (INTERNET_SCHEME_HTTPS == ci.uc.nScheme) ? HTTPS_REQUEST_FLAGS : HTTP_REQUEST_FLAGS;
        hRequest = ::HttpOpenRequest(hConnect, _T("GET"), ci.szUrlPath, NULL, NULL, NULL, dwFlags, 0);
        if (NULL == hRequest)
        {
            _PrintError(_T("HttpOpenRequest"));
            break;
        }

        // 设置请求文件数据范围
        if (!_SetRequestDataRange(hRequest, pPackageInfo->ullBeginPos, pPackageInfo->ullBeginPos + pPackageInfo->ullContentLength - 1, true))
        {
            _PrintError(_T("SetContentRange"));
            break;
        }

        // 发送请求
        if (!_SendRequest(hRequest, nullptr, 0, nullptr, 0))
        {
            _PrintError(_T("SendRequest"));
            break;
        }

        // 查询文件大小
        if (!_QueryContentLength(hRequest, &ullContentLength))
        {
            _PrintError(_T("QueryContentLength"));
            break;
        }

        if (NULL != pPackageInfo->lpBuffer)
        {
            if (!_InternetReadToBuffer(hRequest, pPackageInfo))
            {
                _PrintError(_T("SaveToBuffer"));
                break;
            }
        }
        else
        {
            if (!_InternetReadToFile(hRequest, pPackageInfo))
            {
                _PrintError(_T("SaveToFile"));
                break;
            }
        }

        bSuccess = true;
    } while (false);

    // 释放资源
    if (hRequest)
    {
        ::InternetCloseHandle(hRequest);
    }

    if (hConnect)
    {
        ::InternetCloseHandle(hConnect);
    }

    if (hInternet)
    {
        ::InternetCloseHandle(hInternet);
    }

    return bSuccess;
}

bool CHttpsDownload::_SetRequestDataRange(
    HINTERNET hRequest, 
    LONGLONG nBegin, 
    LONGLONG nEng, 
    bool isHasEnd
)
{
    TCHAR szBuf[MAX_PATH] = { 0 };

    if (isHasEnd)
    {
        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("Range:bytes=%lld-%lld"), nBegin, nEng);
    }
    else
    {
        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("Range:bytes=%lld-"), nBegin);
    }

    return ::HttpAddRequestHeaders(hRequest, szBuf, (DWORD)_tcslen(szBuf), 0);
}

bool CHttpsDownload::_InternetReadToBuffer(
    HINTERNET hRequest, 
    LPDOWNLOAD_PACKAGE_INFO lpDownloadPackage
)
{
    LPBYTE lpBuffer = NULL;
    LPBYTE lpBufPos = (LPBYTE)lpDownloadPackage->lpBuffer;
    bool bDownloadFinish = false;
    bool bRet = false;

    // 分配下载缓冲
    lpBuffer = (LPBYTE)::HeapAlloc(::GetProcessHeap(), 0, DOWNLOAD_BLOCK_SIZE);
    if (NULL == lpBuffer)
    {
        _PrintError(_T("HeapAlloc"));
        goto L_Cleanup;
    }

    do
    {
        // 读取下载的数据到缓冲区
        DWORD dwRead = 0;
        bRet = ::InternetReadFile(hRequest, lpBuffer, DOWNLOAD_BLOCK_SIZE, &dwRead);
        if (false == bRet)
        {
            lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFailed;
            _PrintError(_T("InternetReadFile"));
            break;
        }

        // 检查下载是否完成
        if (0 == dwRead)
        {
            bDownloadFinish = true;
            lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFinish;
            break;
        }

        // 写入到缓冲
        memcpy(lpBufPos, lpBuffer, dwRead);
        lpBufPos += dwRead;

        // 记录当前线程已下载数据量
        lpDownloadPackage->ullDownloaded += dwRead;

        // 中断下载检查
        if (m_bAbort)
        {
            lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadCancel;
            break;
        }

    } while (bRet);

L_Cleanup:

    if (lpBuffer)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpBuffer);
    }

    return bDownloadFinish;
}

bool CHttpsDownload::_InternetReadToResponse(
    HINTERNET hRequest, 
    std::string& strResponse
)
{
    LPBYTE lpBuffer = NULL;
    bool bDownloadFinish = false;
    bool bRet = false;

    // 分配下载缓冲
    lpBuffer = (LPBYTE)::HeapAlloc(::GetProcessHeap(), 0, DOWNLOAD_BLOCK_SIZE);
    if (NULL == lpBuffer)
    {
        _PrintError(_T("HeapAlloc"));
        goto L_Cleanup;
    }

    do
    {
        // 读取下载的数据到缓冲区
        DWORD dwRead = 0;
        bRet = ::InternetReadFile(hRequest, lpBuffer, DOWNLOAD_BLOCK_SIZE, &dwRead);
        if (false == bRet)
        {
            _PrintError(_T("InternetReadFile"));
            break;
        }

        // 检查下载是否完成
        if (0 == dwRead)
        {
            bDownloadFinish = true;
            break;
        }

        strResponse += std::string((const char*)lpBuffer, dwRead);

        // 中断下载检查
        if (m_bAbort)
        {
            break;
        }

    } while (bRet);

L_Cleanup:

    if (lpBuffer)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpBuffer);
    }

    return bDownloadFinish;
}

bool CHttpsDownload::_InternetReadToFile(
    HINTERNET hRequest, 
    LPDOWNLOAD_PACKAGE_INFO lpDownloadPackage
)
{
    LPBYTE lpBuffer = NULL;
    LARGE_INTEGER liDistanceToMove = { 0 };
    HANDLE hFile = INVALID_HANDLE_VALUE;
    bool bDownloadFinish = false;
    bool bRet = false;

    do
    {
        // 共享读写 创建/打开 文件, 多线程读写
        hFile = ::CreateFile(lpDownloadPackage->strFileName.c_str(),
            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_ARCHIVE,
            NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            _PrintError(_T("CreateFile"));
            break;
        }

        // 分配下载缓冲
        lpBuffer = (LPBYTE)::HeapAlloc(::GetProcessHeap(), 0, DOWNLOAD_BLOCK_SIZE);
        if (NULL == lpBuffer)
        {
            _PrintError(_T("HeapAlloc"));
            break;
        }

        liDistanceToMove.QuadPart = lpDownloadPackage->ullBeginPos;
        ::SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN);

        do
        {
            // 读取下载的数据到缓冲区
            DWORD dwRead = 0;
            bRet = ::InternetReadFile(hRequest, lpBuffer, DOWNLOAD_BLOCK_SIZE, &dwRead);
            if (false == bRet)
            {
                lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFailed;
                _PrintError(_T("InternetReadFile"));
                break;
            }

            // 检查下载是否完成
            if (0 == dwRead)
            {
                bDownloadFinish = true;
                lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFinish;
                break;
            }

            // 写入到文件
            DWORD dwWritten = 0;
            if (!::WriteFile(hFile, lpBuffer, dwRead, &dwWritten, NULL))
            {
                lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadFailed;
                _PrintError(_T("WriteFile"));
                break;
            }

            // 记录当前线程已下载数据量
            lpDownloadPackage->ullDownloaded += dwWritten;

            // 中断下载检查
            if (m_bAbort)
            {
                lpDownloadPackage->dwStatus = DOWNLOAD_PACKAGE_INFO::DownloadStatus::DS_DownloadCancel;
                break;
            }

        } while (bRet);

    } while (false);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    if (lpBuffer)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpBuffer);
    }

    return bDownloadFinish;
}

bool CHttpsDownload::_IsSupportAcceptRanges(HINTERNET hRequest)
{
    TCHAR szResponseBuffer[MAX_PATH] = { 0 };
    DWORD dwBufferLength = sizeof(szResponseBuffer);

    // 查询资源大小
    if (!::HttpQueryInfo(hRequest, HTTP_QUERY_ACCEPT_RANGES, szResponseBuffer, &dwBufferLength, NULL))
    {
        _PrintError(_T("_IsSupportAcceptRanges HttpQueryInfo"));
        return false;
    }

    if (0 == _tcsicmp(szResponseBuffer, _T("bytes")))
    {
        return true;
    }

    return false;
}

bool CHttpsDownload::_QueryContentLength(
    HINTERNET hRequest, 
    PULONGLONG lpUllContentLength
)
{
    TCHAR szResponseBuffer[MAX_PATH] = { 0 };
    DWORD dwBufferLength = sizeof(szResponseBuffer);

    // 查询资源大小
    if (!::HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, szResponseBuffer, &dwBufferLength, NULL))
    {
        m_bHasSize = false;
        _PrintError(_T("QueryContentLength HttpQueryInfo"));
        *lpUllContentLength = (ULONGLONG)(-1);
        return true;
    }

    m_bHasSize = true;
    *lpUllContentLength = _tcstoll(szResponseBuffer, NULL, 10);

    return true;
}

long CHttpsDownload::_GetStatusCodes(HINTERNET hRequest)
{
    DWORD dwRespCode = 0;
    DWORD dwSize = sizeof(dwRespCode);

    // 查询请求状态码
    if (!::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwRespCode, &dwSize, NULL))
    {
        return dwRespCode;
    }

    return dwRespCode;
}

bool CHttpsDownload::_SendRequest(
    HINTERNET hRequest, 
    LPCTSTR lpszHeaders, 
    DWORD dwHeadersLength, 
    LPVOID lpData, 
    DWORD dwSize
)
{
    DWORD dwFlags = 0;
    DWORD dwFlagsSize = sizeof(dwFlags);
    
    // 将指定的请求发送到 HTTP 服务器
    if (::HttpSendRequest(hRequest, lpszHeaders, dwHeadersLength, lpData, dwSize))
    {
        return true;
    }

    // 检查错误是否为未知证书颁发机构
    if (ERROR_INTERNET_INVALID_CA != ::GetLastError())
    {
        return false;
    }

    // 获取INTERNET_OPTION_SECURITY_FLAGS标志 
    if (!::InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &dwFlagsSize))
    {
        _PrintError(_T("InternetQueryOption"));
        return false;
    }
    
    // 忽略未知证书颁发机构问题。
    dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    if (!::InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags)))
    {
        _PrintError(_T("InternetSetOption"));
        return false;
    }
    
    // 再次发送请求
    return ::HttpSendRequest(hRequest, lpszHeaders, dwHeadersLength, lpData, dwSize);
}

bool CHttpsDownload::_CrackUrl(
    const _tstring& strUrl, 
    LPDOWNLOAD_URL_COMPONENTS lpUci
)
{
    // 将 URL 分解到其组件部件中
    if (!::InternetCrackUrl(strUrl.c_str(), 0, 0, &lpUci->uc))
    {
        _PrintError(_T("InternetCrackUrl"));
        return false;
    }

    if (0 < ::_tcslen(lpUci->uc.lpszExtraInfo))
    {
        ::_tcscat_s(lpUci->uc.lpszUrlPath, _countof(lpUci->szUrlPath), lpUci->uc.lpszExtraInfo);
    }

    return true;
}

void CHttpsDownload::_PrintError(LPCTSTR lpszError) const
{
    ::_tprintf(_T("[Error] %s LastError[%d]\n"), lpszError, ::GetLastError());
}

void CHttpsDownload::_PrintWarn(LPCTSTR lpszError)  const
{
    ::_tprintf(_T("[Warn] %s LastError[%d]\n"), lpszError, ::GetLastError());
}

void CHttpsDownload::_InternetStatusCallback(
    HINTERNET hInternet, 
    DWORD_PTR dwContext, 
    DWORD dwInternetStatus, 
    LPVOID lpvStatusInformation, 
    DWORD dwStatusInformationLength
)
{
    UNREFERENCED_PARAMETER(hInternet);
    UNREFERENCED_PARAMETER(dwContext);
    UNREFERENCED_PARAMETER(dwStatusInformationLength);

    switch (dwInternetStatus)
    {
    //由 InternetConnect 用来指示它已创建新句柄。
    case INTERNET_STATUS_HANDLE_CREATED:
    {

    }
    break;
    // 此句柄值已终止。 pvStatusInformation 包含正在关闭的句柄的地址。 lpvStatusInformation 参数包含正在关闭的句柄的地址。
    case  INTERNET_STATUS_HANDLE_CLOSING:
    {

    }
    break;
    // 异步操作已完成。 lpvStatusInformation 参数包含INTERNET_ASYNC_RESULT结构的地址。
    case  INTERNET_STATUS_REQUEST_COMPLETE:
    {
        if (ERROR_SUCCESS == ((LPINTERNET_ASYNC_RESULT)
        (lpvStatusInformation))->dwError)
        {

        }
        else
        {

        }
    }
    break;
    }
}
