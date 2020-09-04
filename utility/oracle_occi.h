#ifndef __ORACLE_OCCI_H__
#define __ORACLE_OCCI_H__

#include "occi.h"                           // for oracle occi库
#include "MyLog.h"

// oracle数据库连接
class connOracle
{
public:
    connOracle(std::string user, std::string password, std::string db)
    {
        initSucceed = false;

        try {
            _env = oracle::occi::Environment::createEnvironment(oracle::occi::Environment::DEFAULT);
            if (NULL == _env) {
                LOG_ERROR("Failed to create oracle env.");
                return;
            }

            _conn = _env->createConnection(user, password, db);
            if (NULL == _conn) {
                LOG_ERROR("Failed to create oracle connection: user: {}, password: {}, db: {}", user, password, db);
                return;
            }

            _statement = _conn->createStatement();
            if (NULL == _statement) {
                LOG_ERROR("Failed to create statement.");
                return;
            }
        } catch (const oracle::occi::SQLException &ex) {
            LOG_ERROR("Failed to init oracle user: {}, password: {}, db: {}, error code: {}, error msg: {}", user, password, db, ex.getErrorCode(), ex.getMessage());
            return;
        }

        initSucceed = true;
    }

    operator bool()
    {
        return initSucceed;
    }

    ~connOracle()
    {
        if (NULL != _statement && NULL != _rset) {
            _statement->closeResultSet(_rset);
            _rset = NULL;
        }

        if (NULL != _conn && NULL != _statement) {
            _conn->terminateStatement(_statement);
            _statement = NULL;
        }

        if (NULL != _env && NULL != _conn) {
            _env->terminateConnection(_conn);
            _conn = NULL;
        }

        if (NULL != _env) {
            oracle::occi::Environment::terminateEnvironment(_env);
            _env = NULL;
        }
    }

    oracle::occi::Connection *getOracleConn()
    {
        return _conn;
    }

    oracle::occi::ResultSet *executeQuery(const std::string &sql)
    {
        try {
            _statement->setSQL(sql);
            _rset = _statement->executeQuery();
        } catch (const oracle::occi::SQLException &ex) {
            LOG_ERROR("Failed to execute query sql: [{}], error code: {}, error msg: {}", sql, ex.getErrorCode(), ex.getMessage());
            return NULL;
        }

        return _rset;
    }

    int executeUpdate(const std::string &sql)
    {
        try {
            _statement->setSQL(sql);
            _statement->executeUpdate();
            _conn->commit();
        } catch (const oracle::occi::SQLException &ex) {
            LOG_ERROR("Failed to execute update sql: [{}], error code: {}, error msg: {}", sql, ex.getErrorCode(), ex.getMessage());

            // 0: 成功 1: 重复的数据库key
            return ex.getErrorCode();
        }

        return 0;
    }

private:
    oracle::occi::Environment *_env = NULL;
    oracle::occi::Connection *_conn = NULL;
    oracle::occi::Statement *_statement = NULL;
    oracle::occi::ResultSet *_rset = NULL;

    bool initSucceed = false;
};

#endif /* __ORACLE_OCCI_H__ */
