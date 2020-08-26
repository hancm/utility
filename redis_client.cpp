#include <stdint.h>
#include <iostream>
#include "MyLog.h"
#include "redis_client.h"

#define REDIS_EX_NOT_CONNECTED  ("Redis is not connected")

static unsigned int APHash(const char *str)
{
    unsigned int hash = 0;
    int i;
    for (i=0; *str; i++) {
        if ((i&  1) == 0) {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        } else {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }
    return (hash&  0x7FFFFFFF);
}

enum
{
    CACHE_TYPE_1,
    CACHE_TYPE_2,
    CACHE_TYPE_MAX,
};

RedisClient::RedisClient()
    : isConnected(false)
{
}

RedisClient::~RedisClient()
{
    isConnected = false;
    xRedis.Release();
}

bool RedisClient::connect(const std::vector<RedisHost> &hosts)
{
    if(isConnected)
    {
        LOG_DEBUG("Redis client has connected.");
        return true;
    }

    if (hosts.empty())
    {
        LOG_ERROR("Empty redis host list.");
        return false;
    }

    size_t i = 0;
    for(i = 0; i < hosts.size() && i < MAXINDEX; i++)
    {
        redisNodeList[i].dbindex = i;
        redisNodeList[i].host = hosts[i].host.c_str();
        redisNodeList[i].port = atoi(hosts[i].port.c_str());
        redisNodeList[i].passwd = hosts[i].passwd.c_str();
        redisNodeList[i].poolsize = 8;
        redisNodeList[i].timeout = 5;
        redisNodeList[i].role = MASTER;
        LOG_DEBUG("Redis host:[{}]", std::string("host=") + hosts[i].host + ", port=" + hosts[i].port + ", passwd=" + hosts[i].passwd + ", database=" + hosts[i].database);
    }

    xRedis.Init(CACHE_TYPE_MAX);
    if(!xRedis.ConnectRedisCache(redisNodeList, i, CACHE_TYPE_1))
    {
        LOG_ERROR("Failed to connect redis.");
        isConnected = false;
        return false;
    }

    isConnected = true;
    return true;
}

void RedisClient::keepAlive()
{
    xRedis.Keepalive();
}

bool RedisClient::createRedisDBIdx(const std::string &key, RedisDBIdx &redis_dbi)
{
    RedisDBIdx dbi(&xRedis);
    if(!dbi.CreateDBIndex(key.c_str(), APHash, CACHE_TYPE_1))
    {
        LOG_ERROR("Failed to create dbi for key: {}, errmsg: {}", key, dbi.GetErrInfo());
        return false;
    }
    redis_dbi = dbi;
    return true;
}

bool RedisClient::set(const string &key, const string &value)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    if(!xRedis.set(redis_dbi, key, value))
    {
        LOG_ERROR("Failed to xredis set for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
       return false;
    }

    return true;
}

bool RedisClient::get(const string &key, string &outValue)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    if(!xRedis.get(redis_dbi, key, outValue))
    {
        LOG_ERROR("Failed to xredis get for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
        return false;
    }

    return true;
}

bool RedisClient::exists(const string &key)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    if(!xRedis.exists(redis_dbi, key))
    {
        LOG_ERROR("Failed to xredis exists for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
        return false;
    }

    return true;
}

bool RedisClient::pexpire(const string &key, unsigned int milliseconds)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    if(!xRedis.pexpire(redis_dbi, key, milliseconds))
    {
        LOG_ERROR("Failed to xredis pexpire for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
        return false;
    }

    return true;
}

bool RedisClient::mget(map<string, string> &keyValuePairs)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    DBIArray dbiArray;
    KEYS keys;
    for(auto &kv : keyValuePairs)
    {
        RedisDBIdx redis_dbi;
        bool bRet = createRedisDBIdx(kv.first, redis_dbi);
        if(!bRet)
        {
            return false;
        }

        dbiArray.push_back(redis_dbi);
        keys.push_back(kv.first);
    }

    ReplyData vdata;
    if(!xRedis.mget(dbiArray, keys, vdata))
    {
        LOG_ERROR("Failed to xredis mget.");
        return false;
    }

    int i = 0;
    for(auto& kv : keyValuePairs)
    {
        kv.second = vdata[i++].str;
    }

    return true;
}

bool RedisClient::mset(const map<string, string> &keyValuePairs)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    DBIArray dbiArray;
    VDATA vdata;
    for(auto const &kv : keyValuePairs)
    {
        RedisDBIdx redis_dbi;
        bool bRet = createRedisDBIdx(kv.first, redis_dbi);
        if(!bRet)
        {
            return false;
        }

        dbiArray.push_back(redis_dbi);
        vdata.push_back(kv.first);
        vdata.push_back(kv.second);
    }

    if(!xRedis.mset(dbiArray, vdata))
    {
        LOG_ERROR("Failed to xredis mset.");
        return false;
    }

    return true;
}

bool RedisClient::mdel(const vector<string> &keys)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    DBIArray dbiArray;
    for(auto const &key : keys)
    {
        RedisDBIdx redis_dbi;
        bool bRet = createRedisDBIdx(key, redis_dbi);
        if(!bRet)
        {
            return false;
        }

        dbiArray.push_back(redis_dbi);
    }

    int64_t count = 0;
    if(!xRedis.del(dbiArray, keys, count))
    {
        LOG_ERROR("Failed to xredis del.");
        return false;
    }

    return true;
}

bool RedisClient::hmget(const string &key, map<string,string> &fieldAndValues)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    if(!exists(key))
    {
        return false;
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    std::vector<std::string> fields;
    for(auto const &pair : fieldAndValues)
    {
        fields.push_back(pair.first);
    }

    ArrayReply arrayReply;
    if(!xRedis.hmget(redis_dbi, key, fields, arrayReply))
    {
        LOG_ERROR("Failed to xredis hmget for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
        return false;
    }

    int i = 0;
    for(auto &pair : fieldAndValues)
    {
        pair.second = arrayReply[i++].str;
    }

    return true;
}

bool RedisClient::hmset(const string& key,
                        const map<string, string> &fieldAndValues)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    VDATA vdata;
    for(auto const &pair : fieldAndValues)
    {
        vdata.push_back(pair.first);
        vdata.push_back(pair.second);
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    if(!xRedis.hmset(redis_dbi, key, vdata))
    {
        LOG_ERROR("Failed to xredis hmset for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
        return false;
    }

    return true;
}

bool RedisClient::hmdel(const string &key, const vector<string> &fields)
{
    if(!isConnected)
    {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    RedisDBIdx redis_dbi;
    bool bRet = createRedisDBIdx(key, redis_dbi);
    if(!bRet)
    {
        return false;
    }

    int64_t count = 0;
    if(!xRedis.hdel(redis_dbi, key, fields, count))
    {
        LOG_ERROR("Failed to xredis hdel for key: {}, errmsg: {}", key, redis_dbi.GetErrInfo());
        return false;
    }

    return true;
}

