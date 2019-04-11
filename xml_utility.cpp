#include <string>
#include <vector>
#include <sstream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "MyLog.h"
#include "xml_utility.h"

namespace XmlUtility
{

/**
 * @brief 查找当前以及下个同级节点为XML_ELEMENT_NODE
 * @param [IN] pstXmlSubnode
 * @return xmlNodePtr
 * @note
 */
static xmlNodePtr FindXmlElementNode(xmlNodePtr pstXmlSubnode)
{
    while (NULL != pstXmlSubnode) {
        if (XML_ELEMENT_NODE == pstXmlSubnode->type) {
            break;
        }
        pstXmlSubnode = pstXmlSubnode->next;
    }

    return pstXmlSubnode;
}

/**
 * @brief 查找当前以及下个同级节点为指定名称的
 * @param [IN] pstXmlSubnode
 * @param [IN] xmlNodeName
 * @return xmlNodePtr
 * @note
 */
static xmlNodePtr FindXmlNodeByName(xmlNodePtr pstXmlSubnode, const std::string &xmlNodeName)
{
    while (NULL != pstXmlSubnode) {
        /* 节点名称 */
        std::string nodeName = (const char*)pstXmlSubnode->name;
        if (nodeName == xmlNodeName)
        {
            break;
        }

        /* 获取下个元素子节点 */
        pstXmlSubnode = FindXmlElementNode(pstXmlSubnode->next);
    }

    return pstXmlSubnode;
}

/**
 * @brief 查找到指定路径的节点
 * @param [IN] pstDoc
 * @param [IN] nodePathList
 * @return xmlNodePtr
 * @note
 */
static xmlNodePtr FindXmlNodeByPath(const xmlDocPtr pstDoc,
                                    const std::string &nodePath)
{
    // '/'分隔字符串解析
    std::vector<std::string> nodePathList;
    std::istringstream ss(nodePath);
    for (std::string item; std::getline(ss, item, '/'); ) {
        if (item.empty()) {
            continue;
        }
        nodePathList.push_back(item);
    }

    /* 生成解析用的二叉树，获取根节点 */
    const xmlNodePtr pstXmlRootnode = xmlDocGetRootElement(pstDoc);
    if (NULL == pstXmlRootnode) {
        return NULL;
    }

    xmlNodePtr xmlNextNode = pstXmlRootnode;
    for (size_t i = 0; i < nodePathList.size(); ++i)
    {
        xmlNextNode = FindXmlNodeByName(xmlNextNode, nodePathList[i]);
        if (NULL == xmlNextNode) {
            LOG_ERROR("Can not find xml node: {}.", nodePathList[i]);
            break;
        }

        if (i == nodePathList.size() - 1) {
            break;
        }
        xmlNextNode = xmlNextNode->children;
    }

    return xmlNextNode;
}

int ParseXmlNodeListInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                         IN const std::string &nodePath,
                         OUT std::vector<std::map<std::string, std::string>> &oNodeInfoMapList)
{
    // 查找的最后节点
    std::string lastNode = nodePath;
    auto pos = nodePath.rfind('/');
    if (std::string::npos != pos) {
        lastNode = nodePath.substr(pos + 1);
    }

    //忽略空白字符，忽略text节点，否则解析将会把空白、回车等解析成text节点
    xmlKeepBlanksDefault(0);

    /* 内存解析方式 */
    const xmlDocPtr pstDoc = xmlParseMemory(pcXMLBuff, iXMLLen);
    if (NULL == pstDoc)
    {
        LOG_ERROR("Failed to parse xml.");
        return -1;
    }

    // 查找到指定路径的子节点
    xmlNodePtr pstXmlSubnode = FindXmlNodeByPath(pstDoc, nodePath);
    if (NULL == pstXmlSubnode) {
        LOG_ERROR("Failed to find xml node by path: {}.", nodePath);
        return -1;
    }

    /* 取所有子节点的内容 */
    while (NULL != pstXmlSubnode)
    {
        xmlNodePtr pstXmlChildNode = pstXmlSubnode->children;

        /**
         * 获取同一级的所有节点信息
         */
        std::map<std::string, std::string> nodeMapInfo;
        while(NULL != pstXmlChildNode)
        {
            /* 取节点内容 */
            const char *pcNodeContent = (char*)xmlNodeGetContent(pstXmlChildNode);
            if (NULL == pcNodeContent)
            {
                xmlFreeDoc(pstDoc);
                return -1;
            }

            /* 节点名称、节点内容 */
            std::string xmlNodeName = (const char*)pstXmlChildNode->name;
            std::string xmlNodeContent = (const char*)pcNodeContent;

            /* 释放由xmlNodeGetContent获取的节点内容指针 */
            xmlFree((void*)pcNodeContent);
            pcNodeContent = NULL;

            /* 插入节点信息 */
            if (!xmlNodeName.empty())
            {
                nodeMapInfo[xmlNodeName] = xmlNodeContent;
            }

            /* 获取同级下个元素节点 */
            pstXmlChildNode = FindXmlElementNode(pstXmlChildNode->next);
        }

        if (!nodeMapInfo.empty()) {
            oNodeInfoMapList.push_back(nodeMapInfo);
        }

        pstXmlSubnode = FindXmlNodeByName(pstXmlSubnode->next, lastNode);
    }

    /* 释放由xmlParseMemory申请的Doc指针 */
    xmlFreeDoc(pstDoc);

    return 0;
}

int ParseXmlNodeInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                     IN const std::string &nodePath,
                     OUT std::map<std::string, std::string> &oNodeInfoMap)
{
    std::vector<std::map<std::string, std::string>> oNodeInfoMapList;
    int iRet = ParseXmlNodeListInfo(pcXMLBuff, iXMLLen, nodePath, oNodeInfoMapList);
    if (0 != iRet) {
        LOG_ERROR("Failed to parse xml node list info.");
        return iRet;
    }

    if (!oNodeInfoMapList.empty()) {
        oNodeInfoMap = oNodeInfoMapList[0];
    }

    return 0;
}

int GetXmlNode(IN const char *pcXMLBuff, IN const int iXMLLen,
               IN const std::string &nodePath,
               OUT std::string &nodeInfo)
{
    std::map<std::string, std::string> oNodeInfoMap;
    int iRet = ParseXmlNodeInfo(pcXMLBuff, iXMLLen, nodePath, oNodeInfoMap);
    if (0 != iRet) {
        LOG_ERROR("Failed to parse xml: {}.");
        return iRet;
    }

    if (oNodeInfoMap.empty()) {
        LOG_ERROR("Failed to find path: {}", nodePath);
        return -1;
    }

    nodeInfo = oNodeInfoMap.begin()->second;
    return 0;
}

} /* namespace XmlUtility */