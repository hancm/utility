#include <string>
#include <map>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "xmlConf.h"

namespace MyUtilityLib {

/**
* 解析XML获取获取所有节点信息
* @param [IN] const char *pcXMLBuff             xml内容
* @param [IN] IN const int iXMLLen              xml长度
* @param [IN] const std::string &strRootnode    xml根节点，用于比较
* @param [OUT] std::vector<TAG_XML_NODE_INFO_S> &oNodeInfoList  XML节点信息列表
* @return int
* - 成功返回     0
* - 失败返回     各错误值
* @Note
*/
int ParseXmlDateInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                     IN const std::string &strRootnode,
                     OUT std::vector<TAG_XML_NODE_INFO_S> &oNodeInfoList)
{
    /* 内存解析方式 */
    const xmlDocPtr pstDoc = xmlParseMemory(pcXMLBuff, iXMLLen);
    if (NULL == pstDoc)
    {
        return -1;
    }

    /* 生成解析用的二叉树，获取根节点 */
    const xmlNodePtr pstXmlRootnode = xmlDocGetRootElement(pstDoc);
    if (NULL == pstXmlRootnode)
    {
        xmlFreeDoc(pstDoc);
        return -1;
    }

    /* 根节点比较 */
    if (0 != xmlStrncmp(pstXmlRootnode->name, (const xmlChar*)strRootnode.c_str(), strRootnode.size()))
    {
        xmlFreeDoc(pstDoc);
        return -1;
    }

    /* 如果比对成功则取根节点的子节点，并找到元素节点 */
    xmlNodePtr pstXmlSubnode = pstXmlRootnode->children;
    while (NULL != pstXmlSubnode)
    {
        if (XML_ELEMENT_NODE == pstXmlSubnode->type)
        {
            break;
        }
        pstXmlSubnode = pstXmlSubnode->next;
    }
    if (NULL == pstXmlSubnode)
    {
        xmlFreeDoc(pstDoc);
        return -1;
    }

     /* 取所有子节点的内容 */
    while(NULL != pstXmlSubnode)
    {
        /**
         * 根据map中存在的节点得到节点内容和属性
         */

        /* 节点值和属性列表 */
        TAG_XML_NODE_INFO_S xmlNodeInfo;

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
        xmlNodeInfo.xmlNodeName = (const char*)pstXmlSubnode->name;
        xmlNodeInfo.xmlNodeContent = (const char*)pcNodeContent;

        /* 释放由xmlNodeGetContent获取的节点内容指针 */
        xmlFree((void*)pcNodeContent);
        pcNodeContent = NULL;

        /**
         * 获取节点属性值
         */
        /* 节点属性列表 */
        xmlAttrPtr attrPtr = pstXmlSubnode->properties;
        while (attrPtr != NULL)
        {
            xmlChar* szAttr = xmlGetProp(pstXmlSubnode, attrPtr->name);
            if (NULL == szAttr)
            {
                xmlFreeDoc(pstDoc);
                return -1;
            }

            xmlNodeInfo.propMap[(const char*)attrPtr->name] = (const char*)szAttr;
            xmlFree(szAttr);
            szAttr = NULL;

            attrPtr = attrPtr->next;
        }

        /* 插入节点信息 */
        oNodeInfoList.push_back(xmlNodeInfo);

        /* 获取下个元素子节点 */
        pstXmlSubnode = pstXmlSubnode->next;
        while (NULL != pstXmlSubnode)
        {
            if (XML_ELEMENT_NODE == pstXmlSubnode->type)
            {
                break;
            }
            pstXmlSubnode = pstXmlSubnode->next;
        }
    }

    /* 释放由xmlParseMemory申请的Doc指针 */
    xmlFreeDoc(pstDoc);

    return 0;
}

} /* End for namespace MyUtilityLib */
