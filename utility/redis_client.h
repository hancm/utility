#pragma once

#include <string>
#include <vector>
#include <map>
#include <atomic>

#include "cpp_redis/cpp_redis"

struct RedisHost
{
    std::string masterName;
    std::vector<std::pair<std::string, int>> hostPortList;
    int         database;
    std::string user;
    std::string passwd;
};

class RedisClient
{
public:
    RedisClient();
    ~RedisClient();

public:
    bool connect(const RedisHost &host);
    bool auth(const std::string &password);
    bool select(int index);
    bool ping();
    void keepAlive();

    bool exists(const std::vector<std::string> &keys);
    bool pexpire(const std::string &key, size_t milliseconds);

    bool set(const std::string &key, const std::string &value);
    bool get(const std::string &key, std::string &outValue);
    bool mset(const std::map<std::string, std::string> &keyValuePairs);
    bool mget(std::map<std::string, std::string> &keyValuePairs);
    bool del(const std::vector<std::string> &keys);

    bool hset(const std::string &key, const std::string &field, const std::string &value);
    bool hget(const std::string &key, const std::string &field, std::string &value);
    bool hmset(const std::string &key, const std::map<std::string, std::string> &fieldAndValues);
    bool hmget(const std::string &key, std::map<std::string, std::string> &fieldAndValues);
    bool hdel(const std::string &key, const std::vector<std::string> &fields);

private:
    bool commit();

private:
    RedisHost _host;
    cpp_redis::client _redisClient;
    std::atomic_bool _isConnected = ATOMIC_VAR_INIT(false);
};
