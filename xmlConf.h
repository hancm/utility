#ifndef __XMLCONF_H__
#define __XMLCONF_H__

#include <string>
#include <vector>
#include <map>

#define IN
#define INOUT
#define OUT

/**
 * @brief 解析XML获取指定路径一级子节点信息
 * @param [IN] pcXMLBuff                xml内容
 * @param [IN] iXMLLen                  xml长度
 * @param [IN] nodePath                 xml节点路径(格式: root/childroot1/childroot2)
 * @param [IN] oNodeInfoMapList         获取路径的所有下级子节点信息
 * @return int
 * @note
 */
int ParseXmlNodeListInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                         IN const std::string &nodePath,
                         OUT std::vector<std::map<std::string, std::string>> &oNodeInfoMapList);

int ParseXmlNodeInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                     IN const std::string &nodePath,
                     OUT std::map<std::string, std::string> &oNodeInfoMap);

int GetXmlNode(IN const char *pcXMLBuff, IN const int iXMLLen,
               IN const std::string &nodePath,
               OUT std::string &nodeInfo);

#endif /* __XMLCONF_H__ */