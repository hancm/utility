#pragma once

#include <string>

class UUID
{
public:
    /**
     * @brief 随机生成UUID
     * @return std::string
     * @note
     * 格式: 550E8400-E29B-11D4-A716-446655440000
     */
    static std::string generateRandom();

    /**
     * @brief 随机生成UUID字符串(没有'-')
     * @return std::string
     * @note
     * 格式: 550E8400E29B11D4A716446655440000
     */
    static std::string generateRandomString();
};

