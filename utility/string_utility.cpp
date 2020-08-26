#include <string>
#include <vector>
#include <sstream>
#include <map>

#include "MyLog.h"

#include "string_utility.h"

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
void Split(const std::string &strSplit, const std::string &strDelim, std::vector<std::string> &vecString)
{
    if (strSplit.empty() || strDelim.empty())
    {
        return;
    }

    /* 查找字符串分隔开始位置 */
    std::size_t iStartPos = strSplit.find_first_not_of(strDelim[0]);
    while (iStartPos != std::string::npos)
    {
        /* 查找分隔字符串结尾 */
        std::size_t iEndPos = strSplit.find(strDelim, iStartPos);
        if (iEndPos != std::string::npos)
        {
            vecString.push_back(strSplit.substr(iStartPos, iEndPos - iStartPos));
        }
        else
        {
            /* 已经到尾部 */
            vecString.push_back(strSplit.substr(iStartPos));
            break;
        }

        /* 查找下一个分隔字符串开头位置 */
        iStartPos = strSplit.find_first_not_of(strDelim[0], iEndPos + 1);
    }
}

/**
 * @brief 获取指定数字字符串列表
 * @param [IN] strSplit             分割字符串: 单个字符: 0、1、3等, 区间字符: 2-5, 逗号分割字符: 0, 1, 3, 2, 7
 * @param [OUT] digitList           分割的数字列表
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetSplitDigitList(const std::string &strSplit, std::vector<int> &digitList)
{
    if (strSplit.empty())
    {
        LOG_ERROR("Empty string.");
        return -1;
    }

    // 只能包含字符: "0123456789,-"
    if (std::string::npos != strSplit.find_first_not_of("0123456789,-"))
    {
        LOG_ERROR("Invalid string format: {}, has not of [0123456789,-].", strSplit);
        return -1;
    }

    /**
     * 不能同时有,-
     */
    size_t dotPos = strSplit.find(",");
    size_t cornetPos = strSplit.find("-");
    if (std::string::npos != dotPos &&
        std::string::npos != cornetPos)
    {
        LOG_ERROR("Invalid string format: {}, can not have [,-] at time.", strSplit);
        return -1;
    }

    // 0,1,3,5
    if (std::string::npos != dotPos)
    {
        std::vector<std::string> vecString;
        Split(strSplit, ",", vecString);
        for (size_t i = 0; i < vecString.size(); ++i)
        {
            digitList.push_back(std::atol(vecString[i].c_str()));
        }
    }
    // 0-5
    else if (std::string::npos != cornetPos)
    {
        std::string first = strSplit.substr(0, cornetPos);
        std::string last = strSplit.substr(cornetPos + 1);
        if (std::string::npos != first.find("-") ||
            std::string::npos != last.find("-"))
        {
            LOG_ERROR("Invalid string format: {}", strSplit);
            return -1;
        }

        int firstInt = std::atol(first.c_str());
        int lastInt = std::atol(last.c_str());
        for (int i = firstInt; i <= lastInt; ++i)
        {
            digitList.push_back(i);
        }
    }
    // 单个数字
    else
    {
        digitList.push_back(std::atol(strSplit.c_str()));
    }

    if (digitList.empty())
    {
        LOG_ERROR("Empty digit list, invalid string: {}", strSplit);
        return -1;
    }
    return 0;
}

/**
 * @brief 检查是否包含非字母、非数字、非下划线("_")的字符
 *        也就是文本必须是[字母、数字、'_']的组合
 * @param [IN] strValue
 * @return bool
 * @note
 */
bool CheckHasNotOfAlnumOrUnderline(const std::string &strValue)
{
    for (size_t i = 0; i < strValue.size(); ++i) {
        // 测试字符是否为英文字母或数字或者下划线
        if (!isalnum((int)strValue[i]) && (int)'_' != (int)strValue[i]) {
            return true;
        }
    }

    return false;
}

/**
 * @brief 检查是否包含非汉字、非空白符、非字母、非数字、非下划线、非英文括号
 *        也就是文本必须是[汉字、空白符、英文字母、数字、下划线、英文括号]的组合
 * @param [IN] text
 * @return bool
 * @note
 */
bool CheckHasNotOfChineseOrAlnumOrUnderline(const std::string &text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        // 汉字
        if (!(text[i] >= 0 && text[i] <= 127)) {
            continue;
        }

        // 测试字符是否为空白符、英文字母或数字、下划线、英文括号
        if (!isspace((int)text[i]) && !isalnum((int)text[i]) && (int)'_' != (int)text[i] &&
            (int)'(' != (int)text[i] && (int)')' != (int)text[i])
        {
            return true;
        }
    }

    return false;
}

bool IsInt(const std::string &text)
{
    return IsValue<int>(text);
}

bool IsDouble(const std::string &text)
{
    return IsValue<double>(text);
}

static void char_to_bitstr(unsigned char c, std::string &bitString)
{
    for (int i = 0; i < 8; i++)
    {
        char bit = ((c & (128 >> i)) != 0) ? '1' : '0';
        bitString.push_back(bit);
    }
}

void StringToBitString(const std::string &charString, std::string &bitString)
{
    for (size_t i = 0; i < charString.size(); ++i)
    {
        char_to_bitstr(charString[i], bitString);
    }
}

static void output_bit(int bit, int &output_bit_count, int &output_value, std::string &charString)
{
    output_value = (output_value << 1) | bit;
    if (++output_bit_count == 8)
    {
        charString.push_back((char)output_value);
        output_value = 0;
        output_bit_count = 0;
    }
}

int BitStringToString(const std::string &bitString, std::string &charString)
{
    charString.clear();
    if (0 != bitString.size() % 8)
    {
        LOG_ERROR("Bit string size must be 8*n.");
        return -1;
    }

    int output_bit_count = 0;
    int output_value = 0;
    for (size_t i = 0; i < bitString.size(); ++i)
    {
        if ('0' != bitString[i] && '1' != bitString[i])
        {
            LOG_ERROR("Invalid value: {}", bitString[i]);
            return -1;
        }
        output_bit(bitString[i] - '0', output_bit_count, output_value, charString);
    }

    return 0;
}

int GetSplitStringMap(const std::string &splitString, const std::string &strDelim, std::map<std::string, std::string> &splitMap)
{
    std::vector<std::string> vecString;
    Split(splitString, strDelim, vecString);
    std::string key;
    for (auto &elemString: vecString)
    {
        size_t pos = elemString.find("=");
        if (std::string::npos == pos)
        {
            LOG_ERROR("Invalid format: {}", elemString);
            return -1;
        }

        std::string key = elemString.substr(0, pos);
        std::string value = elemString.substr(pos + 1);
        splitMap[key] = std::move(value);
    }

    return 0;
}

std::string URLEncode(const char *src, size_t srcLen)
{
    static const char *dont_escape = "._-$,;~()";
    static const char *hex = "0123456789abcdef";

    std::string dst;
    for (; srcLen > 0; src++, srcLen--)
    {
        if (std::isalnum(*(const unsigned char *)src) ||
            std::strchr(dont_escape, *(const unsigned char *)src) != NULL)
        {
            dst.push_back(*src);
        }
        else
        {
            dst.push_back('%');
            dst.push_back(hex[(*(const unsigned char *)src) >> 4]);
            dst.push_back(hex[(*(const unsigned char *)src) & 0xf]);
        }
    }

    return dst;
}

std::string URLDecode(const char *src, size_t srcLen, bool isFormUrlEncoded)
{
    int i = 0;
    int j = 0;
    int a = 0;
    int b = 0;
#define HEXTOI(x) (std::isdigit(x) ? x - '0' : x - 'W')

    std::string dst;
    for (; i < (int)srcLen; i++, j++)
    {
        if (i < (int)srcLen - 2 && src[i] == '%' &&
            std::isxdigit(*(const unsigned char *)(src + i + 1)) &&
            std::isxdigit(*(const unsigned char *)(src + i + 2)))
        {
            a = std::tolower(*(const unsigned char *)(src + i + 1));
            b = std::tolower(*(const unsigned char *)(src + i + 2));
            dst.push_back((char)((HEXTOI(a) << 4) | HEXTOI(b)));
            i += 2;
        }
        else if (isFormUrlEncoded && src[i] == '+')
        {
            dst.push_back(' ');
        }
        else
        {
            dst.push_back(src[i]);
        }
    }

    if (i != (int)srcLen)
    {
        return "";
    }
    return dst;
}

/**
 * Helper method for converting strings with non-ascii characters to the URI format (%HH for each non-ascii character).
 *
 * Not converting:
 * (From RFC  RFC 3986 "URI Generic Syntax")
 * unreserved    = ALPHA / DIGIT / “-” / “.” / “_” / “~”
 * gen-delims = “:” / “/” / “?” / “#” / “[” / “]” / “@”
 * sub-delims    = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
 *
 *  3.3. Path
 * pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
 * We also encode sub-delims and ":" "@" to be safe
 *
 * @param str_in the string to be converted
 * @return the string converted to the URI format
 */
std::string URLPathEncode(const std::string &path)
{
    static std::string unreserved = "-._~/";
    std::ostringstream dst;
    for(std::string::const_iterator i = path.begin(); i != path.end(); ++i)
    {
        if(((*i >= 'A' && *i <= 'Z') || (*i >= 'a' && *i <= 'z') || (*i >= '0' && *i <= '9')) ||
            unreserved.find(*i) != std::string::npos)
        {
            dst << *i;
        }
        else
        {
            dst << '%' << std::hex << std::uppercase << (static_cast<int>(*i) & 0xFF);
        }
    }
    return dst.str();
}

std::string URLPathDecode(const std::string &path)
{
    std::string ret;
    char data[] = "0x00";
    std::string::const_iterator i = path.begin();
    for(; i != path.end(); ++i)
    {
        if(*i == '%' && (std::distance(i, path.end()) > 2) && std::isxdigit(*(i+1)) && std::isxdigit(*(i+2)))
        {
            data[2] = *(++i);
            data[3] = *(++i);
            ret += static_cast<char>(std::strtoul(data, 0, 16));
        }
        else
        {
            ret += *i;
        }
    }

    if (i != path.end())
    {
        return "";
    }

    return ret;
}

} /* namespace string */
} /* namespace util */
