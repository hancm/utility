#include "soci/soci.h"
#include "soci/mysql/soci-mysql.h"

#include "MyLog.h"
#include "soci_utility.h"

/**
 * 头文件: -I.../soci/
 * 库文件: -lsoci_mysql -lsoci_core -lmysqlclient
 */

namespace util
{

/**
 * soci连接信息
 */
static int g_pool_size = 10;
soci::connection_pool g_sociConnPool(g_pool_size);

#define MYLOG_NAME_MYSQL_KEEPALIVE      ("MysqlKeepAlive")

static void mysqlPingFailedReconnect(soci::session &sql)
{
    soci::mysql_session_backend *sessionBackEnd = static_cast<soci::mysql_session_backend*>(sql.get_backend());
    if (0 != mysql_ping(sessionBackEnd->conn_)) {
        int errNum = mysql_errno(sessionBackEnd->conn_);
        std::string errMsg = mysql_error(sessionBackEnd->conn_);
        MYLOG_ERROR(MYLOG_NAME_MYSQL_KEEPALIVE, "Can not connect database error: [{}]:[{}], reconnect", errNum, errMsg);
        try {
            sql.close();
            sql.reconnect();
            sql << "set names utf8;";
        } catch (const soci::soci_error &e) {
            MYLOG_ERROR(MYLOG_NAME_MYSQL_KEEPALIVE, "Soci error: [{}].", e.what());
        }
    } else {
        MYLOG_DEBUG(MYLOG_NAME_MYSQL_KEEPALIVE, "Succeed to mysql ping.");
    }
}

/**
 * @brief mysql心跳包线程
 * @return void
 * @note
 */
static void mysqlKeepAliveThread()
{
    std::thread([]()
    {
        while (true)
        {
            // 休眠单位秒
            sleep(30);

            // 循环保活
            for (int i = 0; i < g_pool_size; ++i)
            {
                try {
                    // 当前会话池位置没有空闲
                    if (!g_sociConnPool.try_pos_lease(i, 1000)) {
                        MYLOG_DEBUG(MYLOG_NAME_MYSQL_KEEPALIVE, "Pool pos:[{}] is not free, continue.", i);
                        continue;
                    }

                    MYLOG_DEBUG(MYLOG_NAME_MYSQL_KEEPALIVE, "Ping pool size: [{}], current pos:[{}]", g_pool_size, i);

                    // database ping保活
                    soci::session &sql = g_sociConnPool.at(i);
                    mysqlPingFailedReconnect(sql);
                    g_sociConnPool.give_back(i);
                } catch (const soci::mysql_soci_error &e) {
                    MYLOG_ERROR(MYLOG_NAME_MYSQL_KEEPALIVE, "Mysql error: {}:{}.", e.err_num_, e.what());
                    continue;
                } catch (const soci::soci_error &e) {
                    MYLOG_ERROR(MYLOG_NAME_MYSQL_KEEPALIVE, "Some other error: {}.", e.what());
                    continue;
                }
            }
        }
    }).detach();
}

bool InitMysqlDatabase(const std::string &host, const std::string &port,
                       const std::string &dbname,
                       const std::string &user, const std::string &password,
                       const std::string &logDir,
                       const std::string &logFileName)
{
    static MyLog MysqlKeepAliveLog(logDir + "/" + logFileName, MYLOG_NAME_MYSQL_KEEPALIVE);

    // 初始化连接池
    for (int i = 0; i < g_pool_size; ++i)
    {
        try {
            soci::session &sql = g_sociConnPool.at(i);
            // mysql数据库
            const std::string mysqlConnectString = std::string("host=") + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + password;
            LOG_DEBUG("Mysql connect string: {}, pool size: {}", mysqlConnectString, g_pool_size);
            sql.open("mysql", mysqlConnectString);

            // 防止中文乱码
            sql << "set names utf8;";
        } catch (const soci::mysql_soci_error &e) {
            LOG_ERROR("Mysql error: {}:{}.", e.err_num_, e.what());
            return false;
        } catch (const soci::soci_error &e) {
            LOG_ERROR("Some other error: {}.", e.what());
            return false;
        }
    }

    mysqlKeepAliveThread();
    return true;
}

} /* namespace util */