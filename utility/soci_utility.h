#ifndef __SOCI_UTILITY_H__
#define __SOCI_UTILITY_H__

#include "soci/soci.h"
#include "soci/mysql/soci-mysql.h"

namespace util
{

// mysql数据库key(主键、唯一键等)重复错误码
#define MYSQL_DUPLICATE_KEY         (1062)

extern soci::connection_pool g_sociConnPool;

bool InitMysqlDatabase(const std::string &host, const std::string &port,
                       const std::string &dbname,
                       const std::string &user, const std::string &password,
                       const std::string &logDir = "/var/log/",
                       const std::string &logFileName = "mysql_keepalive.log");

} /* namespace util */

#endif /* __SOCI_UTILITY_H__ */