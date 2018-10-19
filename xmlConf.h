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
 * @param [IN] strRootnode              xml节点路径(格式: root/childroot1/childroot2)
 * @param [IN] oNodeInfoMap
 * @return int
 * @note
 */
int ParseXmlDateInfo(IN const char *pcXMLBuff, IN const int iXMLLen,
                     IN const std::string &strRootnode,
                     OUT std::map<std::string, std::string> &oNodeInfoMap);

#endif /* __XMLCONF_H__ */