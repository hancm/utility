#pragma once

#include <string>

class UUID
{
public:
    static std::string generateRandom();
    static std::string generateRandomString();
};

