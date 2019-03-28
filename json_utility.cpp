#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"

#include "MyLog.h"
#include "json_utility.h"

namespace JsonUtility
{

int GetJsonInfoList(const char *pcJsonBuff, int iJsonLen,
                    std::map<std::string, std::string> &oNodeInfoMap)
{
    static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };

    rapidjson::Document document;
    document.Parse(pcJsonBuff, iJsonLen);
    if (document.HasParseError())
    {
        LOG_ERROR("Parse error: ({}:{}) {}.",
                  document.GetParseError(), document.GetErrorOffset(),
                  rapidjson::GetParseError_En(document.GetParseError()));
        return -1;
    }

    if (!document.IsObject())
    {
        LOG_ERROR("Json root is not object.");
        return -1;
    }

    for (auto &iter: document.GetObject())
    {
        const char *name = iter.name.GetString();
        if (iter.value.IsString())
        {
            const char *value = iter.value.GetString();
            oNodeInfoMap[name] = value;
        }
        else if (iter.value.IsNumber())
        {
            double dValue = iter.value.GetDouble();
            std::string dString = std::to_string(dValue);
            oNodeInfoMap[name] = dString;
        }
        else if (iter.value.IsArray())
        {
            std::string strArray;
            for (auto &v: iter.value.GetArray())
            {
                if (v.IsString())
                {
                    strArray = strArray + v.GetString() + ",";
                }
                else if (v.IsNumber())
                {
                    double dv = v.GetDouble();
                    std::string dStr = std::to_string(dv);
                    strArray += dStr + ",";
                }
                else
                {
                    LOG_DEBUG("Name: {} type: {} is continue.", name, kTypeNames[v.GetType()]);
                    continue;
                }
            }

            if (strArray.empty())
            {
                continue;
            }
            oNodeInfoMap[name] = strArray;
        }
        else
        {
            LOG_DEBUG("Name: {} type: {} is continue.", name, kTypeNames[iter.value.GetType()]);
            continue;
        }
    }

    return 0;
}

}