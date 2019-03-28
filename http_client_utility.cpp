#include <string>

#include "curl/curl.h"
#include "MyLog.h"
#include "http_client_utility.h"

#define IN
#define INOUT
#define OUT

namespace HttpClientUtility
{

class CurlInitUtil
{
public:
    CurlInitUtil()
    {
        /**
         * 设置全局libcurl环境信息
         * 进程有且只能调用一次，非线程安全的。
         * 需要在主线程完成初始化, 若是没有被调用，则在curl_easy_init会设置一个默认模式，
         * 这样可能会导致多次调用发生错误。
         * CURL_GLOBAL_ALL: 一般是好的设置方式，初始化SSL、WIN32
         */
        (void)curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlInitUtil()
    {
        /* 清理全局libcurl环境信息 */
        curl_global_cleanup();
    }
};
static CurlInitUtil curlInitUtil;

/**
 * @brief 接收数据的回调函数
 * @param [IN] pRecv        接收数据的缓冲区
 * @param [IN] iBlockNum    接收的数据块的数目
 * @param [IN] iBlockSize   接收的数据块的大小
  * @param [IN] pUserData   用户参数
 * @return - size_t
 * @attention 可能分多次接收完所有的数据
 */
static size_t WriteCallback(IN char *pRecv, IN size_t iBlockNum, IN size_t iBlockSize, IN void *pUserData)
{
    /* 用户参数 */
    std::string &userData = *(std::string*)pUserData;

    /* 数据大小 */
    size_t iDataLen = iBlockNum * iBlockSize;
    userData.append(pRecv, iDataLen);
    LOG_DEBUG("Block num: {}, Block size: {}, Total size: {}.", iBlockNum, iBlockSize, userData.size());
    return iBlockNum * iBlockSize;
}

int HttpRequestWithPostAndHeader(const char *url, const char *customHeader,
                                 const char *postFiledData, int postFiledDataLen,
                                 std::string &responseData)
{
    /**
     * libcurl的同步阻塞方式
     * 返回的easy handle只能在同一个线程中被调用，不能在多个线程使用。
     * 多个线程时，每个线程都必须创建自己的handle。
     */
    CURL *pCurlHandle = curl_easy_init();
    if (NULL == pCurlHandle)
    {
        LOG_ERROR("Failed to curl_easy_init.");
        return -1;
    }

    /**
     * 设置头信息
     */
     struct curl_slist *chunk = NULL;
     if (NULL != customHeader && '\0' != *customHeader)
     {
         chunk = curl_slist_append(chunk, customHeader);
         if (NULL == chunk) {
             LOG_ERROR("Failed to curl slist append.");
             return -1;
         }
         curl_easy_setopt(pCurlHandle, CURLOPT_HTTPHEADER, chunk);
     }

    /**
     * libcurl设置合适的操作选项
     * 所有的选项都用同一个handle,根据不同的选项设置不同的参数。
     * 函数内部会保留参数的副本，参数可以随后释放。
     */
    curl_easy_setopt(pCurlHandle, CURLOPT_URL, url);

    if (NULL != postFiledData && '\0' != *postFiledData && 0 < postFiledDataLen)
    {
        curl_easy_setopt(pCurlHandle, CURLOPT_POST, 1L);
        curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDS, postFiledData);
        curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDSIZE, postFiledDataLen);
    }

    /**
     * 设置回调函数
     * 没有设置则默认输出到stdout
     * 需要接收的数据可能分多次接收完成
     */
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);

    /**
     * 给回调函数传递用户参数
     */
    (void)curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, &responseData);

    /**
     * 连接到远程主机，执行相应的命令，等待返回结果。
     * 会调用先前设置的回调函数接受数据，若没有设置则输出到标准输出。
     * 一次可能只接收一个字节或者很多字节，libcurl会尽可能多的分发数据。
     */
    CURLcode curlRet = curl_easy_perform(pCurlHandle);
    if(curlRet != CURLE_OK)
    {
        LOG_ERROR("Faied to curl easy perform, error: {}", curl_easy_strerror(curlRet));
        return -1;
    }

    /* free the custom headers */
    if (NULL != chunk)
    {
        curl_slist_free_all(chunk);
    }

    /* 清理easy handle */
    curl_easy_cleanup(pCurlHandle);

    return 0;
}

} /* namespace HttpClient */