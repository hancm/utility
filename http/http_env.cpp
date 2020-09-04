#include <string>
#include <map>
#include <sstream>

#include "MyLog.h"
#include "pugixml_utility.h"
#include "http_env.h"

env::TAG_CONFIG_INFO_S env::g_config;

namespace env
{

int InitLog(const std::string &confPath)
{
    util::PugiXml pugiXml(confPath);
    if (!pugiXml) {
        LOG_ERROR("Failed to read config path: {}", confPath);
        return -1;
    }

    std::string logDir = pugiXml.getNodeString("/httpConf/logDir");
    if (logDir.empty()) {
        LOG_ERROR("Empty log dir.");
        return -1;
    }

    static util::MyLog MLog(logDir + "/http.log");
    static util::MyLog HttpStatusLog(logDir + "/http_status.log", MYLOG_NAME_HTTPSTATUS);
    static util::MyLog MysqlKeepAliveLog(logDir + "/http_mysql_keepalive.log", MYLOG_NAME_MYSQL_KEEPALIVE);
    static util::MyLog RedisKeepAliveLog(logDir + "/http_redis_keepalive.log", MYLOG_NAME_REDISKEEPALIVE);
    return 0;
}

int InitConfig(const std::string &confPath, TAG_CONFIG_INFO_S &config)
{
    /**
     * 读取配置文件
     */
    util::PugiXml pugiXml(confPath);
    if (!pugiXml) {
        LOG_ERROR("Failed to read xml config file: {}", confPath);
        return -1;
    }

    /**
     * 读取xml日志目录
     */
    config.logDir = pugiXml.getNodeString("/httpConf/logDir");
    if (config.logDir.empty()) {
        LOG_ERROR("Faild to get logDir.");
        return -1;
    }
    LOG_DEBUG("Log dir: {}", config.logDir);

    /**
     * civetweb http配置
     */
    std::map<std::string, std::string> httpNodeInfoMap;
    if (0 != pugiXml.getNodeMap("/httpConf/http", httpNodeInfoMap)) {
        LOG_ERROR("Faild to get http.");
        return -1;
    }

    // 重新转换配置文件
    httpNodeInfoMap["access_log_file"] = config.logDir + "/" + httpNodeInfoMap["access_log_file"];
    httpNodeInfoMap["error_log_file"] = config.logDir + "/" + httpNodeInfoMap["error_log_file"];

    config.httpConf.clear();
    for (auto &iter: httpNodeInfoMap) {
        config.httpConf.push_back(iter.first);
        config.httpConf.push_back(iter.second);
        LOG_DEBUG("Civetweb conf: {} = {}", iter.first, iter.second);
    }

    /**
     * http status
     */
    config.httpStatus = pugiXml.getNodeString("/httpConf/httpStatus");
    if (config.httpStatus.empty()) {
        LOG_ERROR("Faild to get httpStatus.");
        return -1;
    }
    LOG_DEBUG("Http status: [{}]", config.httpStatus);

    /**
     * 数据库配置
     */
    config.dbType = pugiXml.getNodeString("/httpConf/database/dbtype");
    config.dbHost = pugiXml.getNodeString("/httpConf/database/host");
    config.dbPort = pugiXml.getNodeString("/httpConf/database/port");
    config.dbName = pugiXml.getNodeString("/httpConf/database/dbname");
    config.dbUser = pugiXml.getNodeString("/httpConf/database/user");
    config.dbPassword = pugiXml.getNodeString("/httpConf/database/password");
    LOG_DEBUG("Datebase conf: type={}, host={}, port={}, dbname={}, user={}, password={}",
              config.dbType,config.dbHost, config.dbPort, config.dbName, config.dbUser, config.dbPassword);

    /**
     * redis配置
     */
    // 是否使用redis
    config.isOnRedis = pugiXml.getNodeString("/httpConf/isOnRedis");

    // redis主机配置
    if ("1" == config.isOnRedis)
    {
        LOG_DEBUG("Using redis.");

        // 哨兵模式下主节点名称
        // 非哨兵模式下为空
        config.masterName = pugiXml.getNodeString("/httpConf/redis/master_name");
        config.database = pugiXml.getNodeInt("/httpConf/redis/database");
        config.user = pugiXml.getNodeString("/httpConf/redis/user");
        config.passwd = pugiXml.getNodeString("/httpConf/redis/password");
        LOG_DEBUG("Redis conf: master_name={}, database={}, user={}, passwd={}",
                  config.masterName, config.database,
                  config.user, config.passwd);

        // 哨兵模式下: 节点列表, ip1:port1,ip2:port2,ip3:port3
        std::string hostPortNodes = pugiXml.getNodeString("/httpConf/redis/nodes");
        std::istringstream iss(hostPortNodes);
        for (std::string item; std::getline(iss, item, ','); )
        {
            if (item.empty()) {
                continue;
            }
            LOG_DEBUG("Redis conf: node={}", item);

            // ip:port
            size_t pos = item.find(":");
            if (std::string::npos == pos) {
                LOG_ERROR("Error redis node format: {}, must [ip:port]", item);
                return -1;
            }

            std::string ip = item.substr(0, pos);
            std::string port = item.substr(pos + 1);
            int nPort = std::atoi(port.c_str());
            LOG_DEBUG("Redis conf: host={}, port={}", ip, nPort);
            config.hostPortList.push_back({ip, nPort});
        }

        // 缓存有效时间
        config.cacheLifeTime = pugiXml.getNodeInt("/httpConf/redis/dataLifeTime");
        LOG_DEBUG("Redis data life time = {}", config.cacheLifeTime);
    }

    return 0;
}

} /* namespace env */

