#include "MyLog.h"
#include "pugixml_utility.h"

namespace util
{

PugiXml::PugiXml(const std::string &xmlData)
{
    _isInitSucceed = false;

    pugi::xml_parse_result result;
    if (0 == strncasecmp(&xmlData[xmlData.size() - 4], ".xml", 4))
    {
        result = _doc.load_file(xmlData.c_str());
    }
    else
    {
        result = _doc.load_buffer(xmlData.c_str(), xmlData.size());
    }

    if (!result)
    {
        LOG_ERROR("Failed to load xml: [{}], error msg: [{}]", xmlData, result.description());
        return;
    }

    _isInitSucceed = true;
}

PugiXml::operator bool()
{
    return _isInitSucceed;
}

std::string PugiXml::getNodeString(const std::string &nodePath)
{
    try
    {
        pugi::xpath_node xpathNode = _doc.select_node(nodePath.c_str());
        if (xpathNode)
        {
            pugi::xml_node xmlNode = xpathNode.node();
            return xmlNode.text().as_string();
        }
    }
    catch (const pugi::xpath_exception &e)
    {
        LOG_ERROR("Failed to get node string value, error msg: {}", e.what());
        return "";
    }

    return "";
}

int PugiXml::getNodeInt(const std::string &nodePath, int defaultValue)
{
    try
    {
        pugi::xpath_node xpathNode = _doc.select_node(nodePath.c_str());
        if (xpathNode)
        {
            pugi::xml_node xmlNode = xpathNode.node();
            return xmlNode.text().as_int(defaultValue);
        }
    }
    catch (const pugi::xpath_exception &e)
    {
        LOG_ERROR("Failed to get node int value, error msg: {}", e.what());
        return defaultValue;
    }

    return defaultValue;
}

double PugiXml::getNodeDouble(const std::string &nodePath, double defaultValue)
{
    try
    {
        pugi::xpath_node xpathNode = _doc.select_node(nodePath.c_str());
        if (xpathNode)
        {
            pugi::xml_node xmlNode = xpathNode.node();
            return xmlNode.text().as_double(defaultValue);
        }
    }
    catch (const pugi::xpath_exception &e)
    {
        LOG_ERROR("Failed to get node double value, error msg: {}", e.what());
        return defaultValue;
    }

    return defaultValue;
}

int PugiXml::getNodeMap(const std::string &nodePath, std::map<std::string, std::string> &nodeInfoMap)
{
    try
    {
        pugi::xpath_node xpathNode = _doc.select_node(nodePath.c_str());
        if  (xpathNode)
        {
            pugi::xml_node xmlNode = xpathNode.node();
            for (pugi::xml_node child: xmlNode.children())
            {
                nodeInfoMap[child.name()] = child.text().as_string();
            }
        }
    }
    catch (const pugi::xpath_exception &e)
    {
        LOG_ERROR("Failed to get node map, error msg: {}", e.what());
        return -1;
    }

    return 0;
}

int PugiXml::getNodesList(const std::string &nodePath, std::vector<std::string> &nodeInfoList)
{
    try
    {
        pugi::xpath_node_set xpathNodeSet = _doc.select_nodes(nodePath.c_str());
        for (pugi::xpath_node xpathNode: xpathNodeSet)
        {
            pugi::xml_node xmlNode = xpathNode.node();
            nodeInfoList.push_back(xmlNode.text().as_string());
        }
    }
    catch (const pugi::xpath_exception &e)
    {
        LOG_ERROR("Failed to get nodes list, error msg: {}", e.what());
        return -1;
    }

    return 0;
}

int PugiXml::getNodesMapList(const std::string &nodePath, std::vector<std::map<std::string, std::string>> &nodeInfoMapList)
{
    try
    {
        pugi::xpath_node_set xpathNodeSet = _doc.select_nodes(nodePath.c_str());
        for (pugi::xpath_node xpathNode: xpathNodeSet)
        {
            pugi::xml_node xmlNode = xpathNode.node();

            std::map<std::string, std::string> nodeMap;
            for (pugi::xml_node child: xmlNode.children())
            {

                nodeMap[child.name()] = child.text().as_string();
            }
            nodeInfoMapList.push_back(std::move(nodeMap));
        }
    }
    catch (const pugi::xpath_exception &e)
    {
        LOG_ERROR("Failed to get nodes map list, error msg: {}", e.what());
        return -1;
    }

    return 0;
}

} /* namespace util */
