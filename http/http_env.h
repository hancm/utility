#ifndef __HTTP_ENV_H__
#define __HTTP_ENV_H__

#include <string>
#include <vector>

#define MYLOG_NAME_HTTPSTATUS           ("HttpStatus")
#define MYLOG_NAME_MYSQL_KEEPALIVE      ("MysqlKeepAlive")
#define MYLOG_NAME_REDISKEEPALIVE       ("RedisKeepAlive")

namespace env
{

typedef struct tag_config_info
{
    // 日志目录
    std::string logDir;

    // http配置
    std::vector<std::string> httpConf;

    // http状态
    std::string httpStatus;

    // 数据库配置
    std::string dbType;
    std::string dbHost;
    std::string dbPort;
    std::string dbName;
    std::string dbUser;
    std::string dbPassword;

    // 是否用redis, 1: 用redis, 其余不用
    std::string isOnRedis;

    // redis
    std::string masterName;
    std::vector<std::pair<std::string, int>> hostPortList;
    int database;
    std::string user;
    std::string passwd;

    // redis数据生存期(默认2小时,单位秒)
    int cacheLifeTime;
} TAG_CONFIG_INFO_S;

extern env::TAG_CONFIG_INFO_S g_config;

int InitLog(const std::string &confPath);
int InitConfig(const std::string &confPath, TAG_CONFIG_INFO_S &config);

} /* namespace env */

#endif /* __HTTP_ENV_H__ */
