#pragma once

#include <string>
#include <vector>

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
                     std::vector<std::string> &vecString);

    /**
     * @brief 检查是否包含非字母、非数字、非下划线("_")的字符
     *        也就是文本必须是[字母、数字、'_']的组合
     * @param [IN] strValue
     * @return bool
     * @note
     */
    bool CheckHasNotOfAlnumOrUnderline(const std::string &strValue);

    /**
     * @brief 检查是否包含非汉字、非字母、非数字、非下划线
     *        也就是文本必须是[汉字、英文字母、数字、下划线]的组合
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

} /* namespace StringUtil */
