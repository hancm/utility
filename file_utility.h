#pragma once

#include <ctime>
#include <string>
#include <vector>

class FileUtility
{
    public:
    static std::string cwd();
    static std::string env(const std::string &varname);
    static bool fileExists(const std::string &path);
    static bool directoryExists(const std::string &path);
    static bool isRelative(const std::string &path);
    static tm *modifiedTime(const std::string &path);

    /**
    * @brief 获取文件扩展名(不包括'.'),一律转换为小写字母
    * @param [IN] path         文件路径
    * @return std::string
    * @note
     * 通过查找最后的'.'字符后面的名称做扩展名
    * /root/123.pdf: 返回pdf
    * /root/123: 没有扩展名为空
    */
    static std::string fileExtension(const std::string &path);

    static size_t fileSize(const std::string &path);

    /**
    * @brief 获取文件名称
    * @param [IN] path     文件路径
    * @return std::string
    * @note
     * 通过查找最后/后面字符为文件名
    * /root/123.pdf: 返回123.pdf
    */
    static std::string fileName(const std::string &path);

    /**
    * @brief 获取文件目录
    * @param [IN] path         文件路径
    * @return std::string
    * @note
     * 查找/之前的字符
    * /root/123.pdf: 返回/root/
    */
    static std::string directory(const std::string &path);

    /**
    * @brief 组合目录和相对路径为新的路径
    * @param [IN] directory         文件路径或者目录
    * @param [IN] relativePath      相对路径
    * @return std::string
    * @note
    * /root/hancm/123.pdf + ./456.pdf: 返回路径/root/hancm/456.pdf
    */
    static std::string path(const std::string &directory, const std::string &relativePath);

    /**
     * @brief 构建full file路径, 格式file:///fullpath使用url编码
     * @param [IN] path
     * @return std::string
     * @note
     */
    static std::string fullPathUrl(const std::string &path);

    /**
     * @brief 创建不重复的临时文件名称
     * @return std::string
     * @note
     */
    static std::string tempFileName();

    /**
     * @brief 递归创建目录, 类似mkdir -p
     * @param [IN] path
     * @return int
     * @note
     */
    static int createDirectory(const std::string &path);

    /**
     * @brief 获取目录下所有文件，支持递归
     * @param [IN] directory            目录
     * @param [IN] files                文件列表
     * @param [IN] bRecurison           false: 不递归子目录，true: 递归子目录
     * @return int
     * @note
     */
    static int listFiles(const std::string &directory, std::vector<std::string> &files, bool bRecurison = false);

    static bool removeFile(const std::string &path);

    /**
     * @brief 转换字符串为URL编码格式，RFC 2396 "URI Generic Syntax"
     * @param [IN] path
     * @return std::string
     * @note
     */
    static std::string toUri(const std::string &path);

    /**
     * @brief 转换字符串为URL编码格式，RFC 3986 "URI Generic Syntax"
     * @param [IN] path
     * @return std::string
     * @note
     */
    static std::string toUriPath(const std::string &path);

    /**
     * @brief 转换URL编码为普通字符串
     * @param [IN] path
     * @return std::string
     * @note
     */
    static std::string fromUriPath(const std::string &path);

    /**
     * @brief 转换十六进制字符串为普通字符
     * @param [IN] in       16进制字符串，格式"AFCD123F"
     * @return 普通char字符串
     * @note
     */
    static std::vector<unsigned char> hexToBin(const std::string &in);

    static int readFileInfo(const std::string &filePath, std::string &fileBuf);
    static int writeFileInfo(const std::string &filePath, const char *fileBuf, size_t fileLen);
    static int copyFile(const char *srcFilePath, const char *dstFilePath);
};

