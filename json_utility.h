#ifndef __JSON_UTILITY__
#define __JSON_UTILITY__

#include <string>
#include <map>

namespace JsonUtility
{

/**
 * @brief 解析JSON数据，支持value类型为string, number, array(string或者nubmer)
 * @param [IN] pcJsonBuff
 * @param [IN] iJsonLen
 * @param [IN] oNodeInfoMap
 * @return int
 * @note 仅支持下面几种格式解析(字符串、整形、浮点数、数组)，数字转化为double字符串
 * {
    "hello": "world",
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4, "5"],
   }
 */
int GetJsonInfoList(const char *pcJsonBuff, int iJsonLen,
                    std::map<std::string, std::string> &oNodeInfoMap);

} /* namespace JsonUtility */
#endif /* __JSON_UTILITY__ */