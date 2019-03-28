#pragma once

#include <string>
#include <map>
#include "xredis/xRedisClient.h"

#define MAXINDEX 5

struct RedisHost
{
    std::string host;
    std::string port;
    std::string user;
    std::string passwd;
};

class RedisClient
{
public:
    RedisClient();
    ~RedisClient();

public:
    bool connect(const std::vector<RedisHost> &hosts);
    void keepAlive();

    //key
    bool exists(const std::string &key);
    bool pexpire(const std::string &key, unsigned int milliseconds);

    // string
    bool get(const std::string &key, std::string &outValue);
    bool set(const std::string &key, const std::string &value);
    bool mget(std::map<std::string, std::string> &keyValuePairs);
    bool mset(const std::map<std::string, std::string> &keyValuePairs);
    bool mdel(const std::vector<std::string> &keys);

    // hash
    bool hmget(const std::string &key, std::map<std::string, std::string> &fieldAndValues);
    bool hmset(const std::string &key, const std::map<std::string, std::string> &fieldAndValues);
    bool hmdel(const std::string &key, const std::vector<std::string> &fields);

private:
    bool createRedisDBIdx(const std::string &key, RedisDBIdx &redis_dbi);

private:
    RedisNode redisNodeList[MAXINDEX];
    xRedisClient xRedis;
    bool isConnected = false;
};
