#ifndef __STRING_UTILITY_H__
#define __STRING_UTILITY_H__

#include <string>
#include <vector>
#include <sstream>

namespace util
{

namespace string
{

/**
 * @brief 字符串分隔函数
 * @param [IN] strSplit         需要分隔的字符串
 * @param [IN] strDelim         分隔字符串,第一个字符为分隔字符，后面是可选的定位字符防止出现不必须的分隔字符
 *                              (",<": 分隔字符',', 分隔字符后面必须为'<'字符)
 * @param [OUT] vecString       分隔后的字符集合
 * @return void
 * @note
 */
void Split(const std::string &strSplit, const std::string &strDelim, std::vector<std::string> &vecString);

/**
 * @brief 获取指定数字字符串列表
 * @param [IN] strSplit             分割字符串: 单个字符: 0、1、3等, 区间字符: 2-5, 逗号分割字符: 0, 1, 3, 2, 7
 * @param [OUT] digitList           分割的数字列表
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetSplitDigitList(const std::string &strSplit, std::vector<int> &digitList);

/**
 * @brief 检查是否包含非字母、非数字、非下划线("_")的字符
 *        也就是文本必须是[字母、数字、'_']的组合
 * @param [IN] strValue
 * @return bool
 * @note
 */
bool CheckHasNotOfAlnumOrUnderline(const std::string &strValue);

/**
 * @brief 检查是否包含非汉字、非空白符、非字母、非数字、非下划线、非英文括号
 *        也就是文本必须是[汉字、空白符、英文字母、数字、下划线、英文括号]的组合
 * @param [IN] text
 * @return bool
 * @note
 */
bool CheckHasNotOfChineseOrAlnumOrUnderline(const std::string &text);

/**
* @brief
* @param [IN] T 检查字符串是否为某种类型
* @param [IN] text
* @return bool
* @note
*/
template<typename T>
bool IsValue(const std::string &text)
{
    std::istringstream iss(text);
    T d;
    if (!(iss >> d)) {
        return false;
    }

    if (!iss.eof()) {
        return false;
    }

    return true;
}

bool IsInt(const std::string &text);

bool IsDouble(const std::string &text);

/**
 * @brief 将普通字符串转换为'01'组成的二进制字符串
 * @param [IN] charString       普通字符串
 * @param [IN] bitString        '01'字符串
 * @return void
 * @note
 * "012" --转换为-- "001100000011000100110010"(16进制0x30 0x31 0x32)
 */
void StringToBitString(const std::string &charString, std::string &bitString);

/**
 * @brief 转换'01'字符串为普通字符串
 * @param [IN] bitString        01字符串, 必须为8的倍数
 * @param [IN] charString       普通字符串
 * @return int
 * @note
 * "001100000011000100110010"(16进制0x30 0x31 0x32) --转换为-- "012"
 */
int BitStringToString(const std::string &bitString, std::string &charString);

/**
 * @brief 根据分隔符分割字符串
 * @param [IN] splitString       需要分割的字符串
 * @param [IN] strDelim          分割字符串
 * @param [OUT] splitMap         分割后的map
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 * 若分割字符strDelim为'&', 分割字符串splitString格式为A=B&C=D&E=F, 则分割后的splitMap为:
 * {{"A", "B"}, {"C", "D"}, {"E", "F"}}
 */
int GetSplitStringMap(const std::string &splitString, const std::string &strDelim, std::map<std::string, std::string> &splitMap);

/**
 * @brief 进行URL编码(From RFC 2396 "URI Generic Syntax")
 * @param [IN] src
 * @param [IN] srcLen
 * @return std::string
 * @note
 */
std::string URLEncode(const char *src, size_t srcLen);

/**
 * @brief URL解码(From RFC 2396 "URI Generic Syntax")
 * @param [IN] src
 * @param [IN] srcLen
 * @param [IN] isFormUrlEncoded
 * @return std::string
 * @note
 */
std::string URLDecode(const char *src, size_t srcLen, bool isFormUrlEncoded = true);

/**
 * @brief 转换字符串为URL编码格式, RFC 3986 "URI Generic Syntax"
 * @param [IN] path
 * @return std::string
 * @note
 */
std::string URLPathEncode(const std::string &path);

/**
 * @brief 转换URL编码为普通字符串, RFC 3986 "URI Generic Syntax"
 * @param [IN] path
 * @return std::string
 * @note
 */
std::string URLPathDecode(const std::string &path);

} /* namespace string */
} /* namespace util */

#endif /* __STRING_UTILITY_H__ */
