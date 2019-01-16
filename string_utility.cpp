#include <string>
#include <vector>
#include <sstream>

#include "MyLog.h"

#include "string_utility.h"

namespace StringUtil
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
void StringSplit(const std::string &strSplit,
                             const std::string &strDelim,
                             std::vector<std::string> &vecString)
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
 * @brief 检查是否包含非汉字、非字母、非数字、非下划线
 *        也就是文本必须是[汉字、英文字母、数字、下划线]的组合
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

        // 测试字符是否为英文字母或数字或者下划线
        if (!isalnum((int)text[i]) && (int)'_' != (int)text[i]) {
            return true;
        }
    }

    return false;
}

bool IsInt(const std::string &text)
{
    return IsValue<int>(text);
}

} /* namespace StringUtil */
