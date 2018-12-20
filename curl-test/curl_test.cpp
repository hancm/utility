#include "curl/curl.h"
#include <stdio.h>
#include <iostream>

#include <stdio.h>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>

#define IN
#define INOUT
#define OUT

using std::cout;
using std::endl;

static std::string g_strRecvbuf;

inline size_t GetFileSize(const char *filePath)
{
    struct stat fileStat = {0};
    int iRet = stat(filePath, &fileStat);
    if (0 != iRet) {
        return 0;
    }

    return fileStat.st_size;
}

int ReadFile(const char *filePath, std::string &fileBuff)
{
    if (NULL == filePath || '\0' == *filePath) {
        printf("File path is empty.\n");
        return -1;
    }

    size_t fileSize = GetFileSize(filePath);
    if (0 >= fileSize) {
        printf("Failed to get file size.\n");
        return -1;
    }

    std::ifstream inFile(filePath, std::ifstream::binary);
    if (!inFile) {
        printf("Failed to open.\n");
        return -1;
    }

    fileBuff.clear();
    fileBuff.resize(fileSize);
    inFile.read((char*)&fileBuff[0], fileSize);
    if (!inFile) {
        printf("Failed to read.\n");
        return -1;
    }
    inFile.close();

    return 0;
}

static int WriteFileInfo(const std::string &strFilePath, const char *fileBuf, size_t fileLen)
{
    std::ofstream outfile(strFilePath.c_str(), std::ios::out | std::ios::binary);
    if (!outfile) {
        return -1;
    }

    if (!outfile.write(fileBuf, fileLen)) {
        outfile.close();
        return -1;
    }

    outfile.close();
    return 0;
}

/**
 * @brief 接收数据的回调函数
 * @param [IN] pRecv        接收数据的缓冲区
 * @param [IN] iBlockNum    接收的数据块的数目
 * @param [IN] iBlockSize   接收的数据块的大小
  * @param [IN] pUserData   用户参数
 * @return - size_t
 * -
 * @author h02199
 * @time   2016/1/28
 * @attention 可能分多次接收完所有的数据
 */
static size_t WriteCallback(IN char *pRecv, IN size_t iBlockNum, IN size_t iBlockSize, IN void *pUserData)
{
    /* 用户参数 */
    char *pData = (char*)pUserData;
    cout << pData << endl;

    /* 数据大小 */
    size_t iDataLen = iBlockNum * iBlockSize;
    g_strRecvbuf.append(pRecv, iDataLen);
	
	cout << "Block num: " << iBlockNum << " Block size: " << iBlockSize << " Total size: " << g_strRecvbuf.size() << endl;
	
	WriteFileInfo("test_tsa.tsa", g_strRecvbuf.c_str(), g_strRecvbuf.size());
    return iBlockNum * iBlockSize;
}

int main(int argc , char **argv)
{
	printf("a.out ip tsq_path\n");
	
	std::string fileBuff;
	ReadFile(argv[2], fileBuff);
	
    /**
     * 设置全局libcurl环境信息
     * 进程有且只能调用一次，非线程安全的。
     * 需要在主线程完成初始化, 若是没有被调用，则在curl_easy_init会设置一个默认模式，
     * 这样可能会导致多次调用发生错误。
     * CURL_GLOBAL_ALL: 一般是好的设置方式，初始化SSL、WIN32
     */
    (void)curl_global_init(CURL_GLOBAL_ALL);

    /**
     * libcurl的同步阻塞方式
     * 返回的easy handle只能在同一个线程中被调用，不能在多个线程使用。
     * 多个线程时，每个线程都必须创建自己的handle。
     */
    CURL *pCurlHandle = curl_easy_init();
    if (NULL == pCurlHandle)
    {
        cout << "Failed to curl_easy_init" << endl;
        return -1;
    }
	
	/**
	 * 设置头信息
	 */
	 struct curl_slist *chunk = NULL;
	 chunk = curl_slist_append(chunk, "Content-Type: application/timestamp-query");
	
    /**
     * libcurl设置合适的操作选项
     * 所有的选项都用同一个handle,根据不同的选项设置不同的参数。
     * 函数内部会保留参数的副本，参数可以随后释放。
     */
	 /* set our custom set of headers */
    curl_easy_setopt(pCurlHandle, CURLOPT_HTTPHEADER, chunk);
	
    /* CURLOPT_URL: 设置需要的URL */
    (void)curl_easy_setopt(pCurlHandle, CURLOPT_URL, argv[1]);
	curl_easy_setopt(pCurlHandle, CURLOPT_POST, 1L);
	curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDS, fileBuff.c_str());
	curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDSIZE, fileBuff.size());
	
    /* 告诉libcurl重定向了URL：1L打开 */ 
    //(void)curl_easy_setopt(pCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);

    /**
     * 设置回调函数
     * 没有设置则默认输出到stdout
     * 需要接收的数据可能分多次接收完成
     */
    (void)curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);

    /**
     * 给回调函数传递用户参数
     * 可以通过修改参数向不同的FILE*打印数据
     */
    //(void)curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, "hancm");

    /**
     * 调试用：输出所有可能的错误信息
     * 正式产品中不应该使用
     */
    //(void)curl_easy_setopt(pCurlHandle, CURLOPT_VERBOSE, 1L);

    /**
     * 连接到远程主机，执行相应的命令，等待返回结果。
     * 会调用先前设置的回调函数接受数据，若没有设置则输出到标准输出。
     * 一次可能只接收一个字节或者很多字节，libcurl会尽可能多的分发数据。
     */
    CURLcode curlRet = curl_easy_perform(pCurlHandle);
    if(curlRet != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curlRet));
    }

	/* free the custom headers */
    curl_slist_free_all(chunk);
	
    /* 清理easy handle */ 
    curl_easy_cleanup(pCurlHandle);

    /* 清理全局libcurl环境信息 */
    curl_global_cleanup();

    return 0;
}