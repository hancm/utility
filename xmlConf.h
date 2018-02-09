#ifndef __XMLCONF_H__
#define __XMLCONF_H__

#include <string>
#include <vector>
#include <map>

#define IN
#define INOUT
#define OUT

namespace MyUtilityLib {

typedef struct tagXMLNodeInfo {
    std::string xmlNodeName;
    std::string xmlNodeContent;
    std::map<std::string, std::string> propMap;
} TAG_XML_NODE_INFO_S;

/**
* 解析XML获取获取所有节点信息
* @param [IN] const char *pcXMLBuff             xml内容
* @param [IN] IN const int iXMLLen              xml长度
* @param [IN] const std::string &strRootnode    xml根节点，用于比较
* @param [OUT] std::vector<TAG_XML_NODE_INFO_S> &oNodeInfoList  XML节点信息列表
* @return int
* - 成功返回     ERR_COMMON_SUCCEED
* - 失败返回     各错误值
* @Note
*/
int ParseXmlDateInfo(IN const char *pcXMLBuff, IN const int iXMLLen,
                     IN const std::string &strRootnode,
                     OUT std::vector<TAG_XML_NODE_INFO_S> &oNodeInfoList);

} /* End for namespace MyUtilityLib */

#endif /* __XMLCONF_H__ */