#ifndef __HTTP_ERROR_H__
#define __HTTP_ERROR_H__

#include <string>
#include <map>

#define HTTP_ERROR_START_VALUE     10000

enum HTTP_ERRNO_ENUM
{
    ERR_COMMON_FAILED = HTTP_ERROR_START_VALUE + 1,

    // 参数解析
    ERR_PARAM_INVALID,
    ERR_PARAM_INVALID_URL,

    // base64参数解析错误
    ERR_BASE64_PARAM_INVALID,

    // 数据库DAO
    ERR_DAO_COMMON_FAILED,
    ERR_DAO_RECORD_NOT_EXISTS,
    ERR_DAO_DUPLICATE_KEY,

    // redis
    ERR_REDIS_COMMON_FAILED,
    ERR_REDIS_KEY_NOT_EXISTS,
    ERR_REDIS_DUPLICATE_KEY,

    // 最大值
    HTTP_ERROR_MAX_VALUE
};

const std::map<int, std::string> g_httpErrnoStringMap =
{
    {ERR_COMMON_FAILED, "操作失败"},

    // 参数解析
    {ERR_PARAM_INVALID, "无效的参数"},
    {ERR_PARAM_INVALID_URL, "无效的URL"},

    // base64参数解析错误
    {ERR_BASE64_PARAM_INVALID, "无效的BASE64参数"},

    // 数据库DAO
    {ERR_DAO_COMMON_FAILED, "数据库操作失败"},
    {ERR_DAO_RECORD_NOT_EXISTS, "数据库记录不存在"},
    {ERR_DAO_DUPLICATE_KEY, "重复的数据库KEY"},

    // REDIS
    {ERR_REDIS_COMMON_FAILED, "REDIS操作失败"},
    {ERR_REDIS_KEY_NOT_EXISTS, "REDIS KEY不存在"},
    {ERR_REDIS_DUPLICATE_KEY, "重复的REDIS KEY"},
};

#endif /* __HTTP_ERROR_H__ */
