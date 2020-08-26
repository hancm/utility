#ifndef __PUGIXML_UTILITY_H__
#define __PUGIXML_UTILITY_H__

#include <string>
#include <map>
#include <vector>

#include "./pugixml/pugixml.hpp"

namespace util
{

class PugiXml
{
public:
    /**
     * @brief 缓存数据或者文件路径解析XML
     * @param [IN] xmlData      XML缓存或者文件路径(必须'.xml'结尾)
     */
    PugiXml(const std::string &xmlData);

    /**
     * @brief 构造函数是否初始化成功
     * @return bool
     * 成功: ture
     * 失败: false
     */
    operator bool();

    /**
     * @brief 获取节点路径的string值
     * @param [IN] nodePath         节点路径(如/root/path1/path2, 不要以'/'结尾)
     * @return std::string          返回的string, 出错返回""
     */
    std::string getNodeString(const std::string &nodePath);

    /**
     * @brief 获取节点路径的int值
     * @param [IN] nodePath             节点路径(如/root/path1/path2, 不要以'/'结尾)
     * @param [IN] defaultValue         默认值, 出错或者失败返回默认值
     * @return int
     * 成功: 节点int值
     * 失败: 默认值
     * @note
     */
    int getNodeInt(const std::string &nodePath, int defaultValue = 0);

    /**
     * @brief 获取节点路径的double值
     * @param [IN] nodePath             节点路径(如/root/path1/path2, 不要以'/'结尾)
     * @param [IN] defaultValue         默认值, 出错或者失败返回默认值
     * @return double
     * 成功: 节点double值
     * 失败: 默认值
     * @note
     */
    double getNodeDouble(const std::string &nodePath, double defaultValue = 0);

    /**
     * @brief 获取节点下的所有子节点信息
     * @param [IN] nodePath              节点PATH
     * @param [OUT] nodeInfoMap          子节点map
     * @return int
     * 成功: 0
     * 失败: -1
     * @note
     *
     * <redis>
     *     <node>
     *         <host>192.168.0.198</host>
     *         <port>6379</port>
     *         <user></user>
     *         <password></password>
     *     </node>
     * </redis>
     * nodePath: /redis/node
     * nodeInfoMap: {{"host", "192.168.0.198"}, {"port", "6379"}, {"user", ""}, {"password", ""}}
     */
    int getNodeMap(const std::string &nodePath, std::map<std::string, std::string> &nodeInfoMap);

    /**
     * @brief 获取节点列表中所有节点信息
     * @param [IN] nodePath             节点PATH
     * @param [OUT] nodeInfoList        节点信息列表
     * @return int
     * 成功: 0
     * 失败: -1
     * @note
     *
     * <redis>
     *      <nodes>
     *          <node>node1</node>
     *          <node>node2</node>
     *          <node>node3</node>
     *      </nodes>
     * </redis>
     * nodePath: /redis/nodes/node
     * nodeInfoList: {"node1", "node2", "node3"}
     */
    int getNodesList(const std::string &nodePath, std::vector<std::string> &nodeInfoList);

    /**
     * @brief 获取节点列表下所有子节点信息
     * @param [IN] nodePath                 节点路径
     * @param [OUT] nodeInfoMapList         所有子节点map信息
     * @return int
     * 成功: 0
     * 失败: -1
     * @note
     * <redis>
     *     <nodes>
     *         <node>
     *             <host>192.168.0.198</host>
     *             <port>6379</port>
     *             <user></user>
     *             <password></password>
     *         </node>
     *         <node>
     *             <host>192.168.0.199</host>
     *             <port>6379</port>
     *             <user></user>
     *             <password></password>
     *         </node>
     *         <dataLifeTime>120</dataLifeTime>
     *     </nodes>
     * </redis>
     * nodePath: /redis/nodes/node
     * nodeInfoMapList: {{"host", "192.168.0.198"}, {"port", "6379"}, {"user", ""}, {"password", ""}},
     *                  {{"host", "192.168.0.199"}, {"port", "6379"}, {"user", ""}, {"password", ""}}
     *
     * nodePath: /redis/nodes
     * nodeInfoMapList: {"node", ""}, {"dataLifeTime", "120"}
     */
    int getNodesMapList(const std::string &nodePath, std::vector<std::map<std::string, std::string>> &nodeInfoMapList);

private:
    bool _isInitSucceed = false;
    pugi::xml_document _doc;
};

} /* namespace util */
#endif /* __PUGIXML_UTILITY_H__ */
