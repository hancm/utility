#ifndef __XMLCONF_H__
#define __XMLCONF_H__

#include <string>
#include <vector>
#include <map>

#define IN
#define INOUT
#define OUT

namespace XmlUtility
{

/**
 * @brief 解析XML获取指定路径一级子节点信息
 * @param [IN] pcXMLBuff                xml内容
 * @param [IN] iXMLLen                  xml长度
 * @param [IN] nodePath                 xml节点路径(格式: root/childroot1/childroot2)
 * @param [IN] oNodeInfoMapList         获取路径的所有下级子节点信息
 * @return int
 * @note
 * <sealConf>
 *  <redis>
        <nodeList>
            <node>
                <host>192.168.0.198</host>
                <port>6379</port>
                <user></user>
                <password></password>
 *          </node>
 *          <node>
                <host>192.168.0.199</host>
                <port>6379</port>
                <user></user>
                <password></password>
            </node>
        </nodeList>
        <dataLifeTime>7200</dataLifeTime>
    </redis>
 * </sealConf>
 * nodePath: sealConf/redis/nodeList
 * oNodeInfoMapList: 获取sealConf/redis/nodeList下的所有node信息
 *
 * nodePath: sealConf/redis/dataLifeTime
 * oNodeInfoMapList: 获取sealConf/redis/dataLifeTime下的信息
 */
int ParseXmlNodeListInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                         IN const std::string &nodePath,
                         OUT std::vector<std::map<std::string, std::string>> &oNodeInfoMapList);

/**
 * @brief  获取指定节点路径下所有内容
 * @param [IN] pcXMLBuff
 * @param [IN] iXMLLen
 * @param [IN] nodePath
 * @param [IN] oNodeInfoMap
 * @return int
 * @note
 */
int ParseXmlNodeInfo(IN const char *pcXMLBuff, IN int iXMLLen,
                     IN const std::string &nodePath,
                     OUT std::map<std::string, std::string> &oNodeInfoMap);

/**
 * @brief 获取指定路径的节点信息
 * @param [IN] pcXMLBuff
 * @param [IN] iXMLLen
 * @param [IN] nodePath
 * @param [IN] nodeInfo
 * @return int
 * @note
 */
int GetXmlNode(IN const char *pcXMLBuff, IN const int iXMLLen,
               IN const std::string &nodePath,
               OUT std::string &nodeInfo);

} /* namespace XmlUtility */
#endif /* __XMLCONF_H__ */