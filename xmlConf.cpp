#include <string>
#include <vector>
#include <sstream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "MyLog.h"
#include "xmlConf.h"

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
 * @brief 查找到指定路径的节点的子节点
 * @param [IN] pstDoc
 * @param [IN] nodePathList
 * @return xmlNodePtr
 * @note
 */
static xmlNodePtr FindChildXmlNodeByPath(const xmlDocPtr pstDoc,
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
        xmlNextNode = xmlNextNode->children;
    }

    return xmlNextNode;
}

int ParseXmlDateInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                     IN const std::string &nodePath,
                     OUT std::map<std::string, std::string> &oNodeInfoMap)
{
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
    xmlNodePtr pstXmlSubnode = FindChildXmlNodeByPath(pstDoc, nodePath);
    if (NULL == pstXmlSubnode) {
        LOG_ERROR("Failed to find xml node by path: {}.", nodePath);
        return -1;
    }

    /* 取所有同级节点的内容 */
    while(NULL != pstXmlSubnode)
    {
        /**
         * 取节点内容
         */
        const char *pcNodeContent = (char*)xmlNodeGetContent(pstXmlSubnode);
        if (NULL == pcNodeContent)
        {
            xmlFreeDoc(pstDoc);
            return -1;
        }

        /* 节点名称、节点内容 */
        std::string xmlNodeName = (const char*)pstXmlSubnode->name;
        std::string xmlNodeContent = (const char*)pcNodeContent;

        /* 释放由xmlNodeGetContent获取的节点内容指针 */
        xmlFree((void*)pcNodeContent);
        pcNodeContent = NULL;

        /* 插入节点信息 */
        if (!xmlNodeName.empty())
        {
            oNodeInfoMap[xmlNodeName] = xmlNodeContent;
        }

        /* 获取同级下个元素节点 */
        pstXmlSubnode = FindXmlElementNode(pstXmlSubnode->next);
    }

    /* 释放由xmlParseMemory申请的Doc指针 */
    xmlFreeDoc(pstDoc);

    return 0;
}
