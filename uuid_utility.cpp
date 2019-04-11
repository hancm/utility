#include <string>

#include <uuid/uuid.h>

#include "uuid_utility.h"

std::string UUID::generateRandom()
{
    uuid_t uuid;
    uuid_generate_random(uuid);
    char buf[50] = {0};
    uuid_unparse(uuid, buf);
    return buf;
}

std::string UUID::generateRandomString()
{
    std::string strID = generateRandom();

    // 删除'-'
    size_t pos;
    while (std::string::npos != (pos = strID.find('-'))) {
        strID.replace(pos, 1, "");
    }

    return strID;
}
