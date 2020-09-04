#include <string>
#include <vector>
#include <map>

#include "cpp_redis/cpp_redis"

#include "MyLog.h"
#include "redis_client.h"

#define REDIS_EX_NOT_CONNECTED  ("Redis is not connected")

RedisClient::RedisClient()
    : _isConnected(false)
{}

RedisClient::~RedisClient()
{
    _isConnected = false;
    _redisClient.disconnect();
}

bool RedisClient::connect(const RedisHost &host)
{
    if (_isConnected) {
        LOG_DEBUG("Redis has connected.");
        return true;
    }

    _host = host;

    try {
        if (!host.masterName.empty() && !host.hostPortList.empty())
        {
            // 哨兵模式
            LOG_DEBUG("Redis sentinel, master name: {}, host port list size: {}", host.masterName, host.hostPortList.size());
            cpp_redis::network::set_default_nb_workers(2);
            for (const std::pair<std::string, int> &hostPort: host.hostPortList) {
                LOG_DEBUG("Redis add sentinel, host: {}, port: {}", hostPort.first, hostPort.second);
                _redisClient.add_sentinel(hostPort.first, hostPort.second);
            }

            _redisClient.connect(host.masterName, [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status) {
                if (status == cpp_redis::client::connect_state::dropped) {
                    LOG_ERROR("Redis client disconnected from host: {} port: {}", host, port);
                }
            });
        }
        else if (!host.hostPortList.empty())
        {
            // 单机模式
            std::string ip = host.hostPortList[0].first;
            int port = host.hostPortList[0].second;
            LOG_DEBUG("Redis no sentinel, host: {}, port: {}", ip, port);
            _redisClient.connect(ip, port);
        }

        if (!_redisClient.is_connected()) {
            LOG_ERROR("Failed to connect redis.");
            _isConnected = false;
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        _isConnected = false;
        return false;
    }
    _isConnected = true;

    if (!host.passwd.empty() && !this->auth(host.passwd)) {
        LOG_ERROR("Failed to auth password: {}", host.passwd);
        _isConnected = false;
        return false;
    }

    if (!this->select(host.database)) {
        LOG_ERROR("Failed to select database: {}", host.database);
        _isConnected = false;
        return false;
    }
    return true;
}

bool RedisClient::commit()
{
    try {
        _redisClient.sync_commit();
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::auth(const std::string &password)
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.auth(password);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string() || ("OK" != reply.as_string())) {
            LOG_ERROR("Reply return is not OK.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;;
}

bool RedisClient::select(int index)
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    if (0 > index || 16 <= index) {
        LOG_ERROR("Invalid index: {}, must be 0..15.", index);
        return false;
    }

    try {
        auto replyFuture = _redisClient.select(index);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string() || ("OK" != reply.as_string())) {
            LOG_ERROR("Reply return is not OK.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;;
}

bool RedisClient::ping()
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.ping();
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            _isConnected = false;
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            _isConnected = false;
            return false;
        }

        if (!reply.is_string() || ("PONG" != reply.as_string())) {
            LOG_ERROR("Reply return is not 1.");
            _isConnected = false;
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        _isConnected = false;
        return false;
    }

    _isConnected = true;
    return true;
}

void RedisClient::keepAlive()
{
    if (!this->ping()) {
        LOG_ERROR("Failed to ping redis.");
        this->connect(_host);
    }
}

bool RedisClient::set(const std::string &key, const std::string &value)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.set(key, value);
        this->commit();

        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string() || ("OK" != reply.as_string())) {
            LOG_ERROR("Reply return is not OK.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::get(const std::string &key, std::string &outValue)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.get(key);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string()) {
            LOG_ERROR("Cpp_redis reply is not string.");
            return false;
        }

        outValue = reply.as_string();
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::exists(const std::vector<std::string> &keys)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.exists(keys);
        this->commit();

        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        int iRet = 0;
        if (!reply.is_integer() || (1 != (iRet = reply.as_integer()))) {
            LOG_DEBUG("Reply return: {}, is not 1.", iRet);
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::pexpire(const std::string &key, size_t milliseconds)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.pexpire(key, milliseconds);
        this->commit();

        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_integer() || (1 != reply.as_integer())) {
            LOG_ERROR("Reply return is not 1.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::mset(const std::map<std::string, std::string> &keyValuePairs)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    std::vector<std::pair<std::string, std::string>> key_vals;
    for (auto &iter: keyValuePairs) {
        key_vals.push_back({iter.first, iter.second});
    }

    try {
        auto replyFuture = _redisClient.mset(key_vals);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string() || ("OK" != reply.as_string())) {
            LOG_ERROR("Reply return is not OK.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::mget(std::map<std::string, std::string> &keyValuePairs)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    std::vector<std::string> keys;
    for (auto &iter: keyValuePairs) {
        keys.push_back(iter.first);
    }

    try {
        auto replyFuture = _redisClient.mget(keys);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_array()) {
            LOG_ERROR("Cpp_redis reply is not array.");
            return false;
        }

        std::vector<std::string> values;
        for (const auto &item : reply.as_array()) {
            if (item && item.is_string()) {
                values.push_back(item.as_string());
            }
        }

        if (keys.size() != values.size()) {
            LOG_ERROR("Key and values size is not equal.");
            return false;
        }

        for (size_t i = 0; i < keys.size(); ++i) {
            keyValuePairs[keys[i]] = values[i];
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::del(const std::vector<std::string> &keys)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    if (!exists(keys)) {
        LOG_DEBUG("Del Key is not exists.");
        return true;
    }

    try {
        auto replyFuture = _redisClient.del(keys);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_integer() || (1 != reply.as_integer())) {
            LOG_ERROR("Reply return is not 1.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::hset(const std::string &key, const std::string &field, const std::string &value)
{
    if (!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.hset(key, field, value);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_integer() || (1 != reply.as_integer())) {
            LOG_ERROR("Reply return is not 1.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::hget(const std::string &key, const std::string &field, std::string &value)
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    if(!exists({key})) {
        LOG_ERROR("Key: {} is not exists.", key);
        return false;
    }

    try {
        auto replyFuture = _redisClient.hget(key, field);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string()) {
            LOG_ERROR("Cpp_redis reply is not string.");
            return false;
        }

        value = reply.as_string();
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::hmset(const std::string &key,
                        const std::map<std::string, std::string> &fieldAndValues)
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    std::vector<std::pair<std::string, std::string>> field_val;
    for (auto &iter: fieldAndValues) {
        field_val.push_back({iter.first, iter.second});
    }

    try {
        auto replyFuture = _redisClient.hmset(key, field_val);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_string() || ("OK" != reply.as_string())) {
            LOG_ERROR("Reply return is not OK.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}


bool RedisClient::hmget(const std::string &key, std::map<std::string, std::string> &fieldAndValues)
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    if(!exists({key})) {
        LOG_ERROR("Key: {} is not exists.", key);
        return false;
    }

    std::vector<std::string> fields;
    for (auto &iter: fieldAndValues) {
        fields.push_back(iter.first);
    }

    try {
        auto replyFuture = _redisClient.hmget(key, fields);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_array()) {
            LOG_ERROR("Cpp_redis reply is not array.");
            return false;
        }

        std::vector<std::string> values;
        for (const auto &item : reply.as_array()) {
            if (item && item.is_string()) {
                values.push_back(item.as_string());
            }
        }

        if (fields.size() != values.size()) {
            LOG_ERROR("Fields and values size is not equal.");
            return false;
        }

        for (size_t i = 0; i < fields.size(); ++i) {
            fieldAndValues[fields[i]] = values[i];
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

bool RedisClient::hdel(const std::string &key, const std::vector<std::string> &fields)
{
    if(!_isConnected) {
        LOG_ERROR(REDIS_EX_NOT_CONNECTED);
        return false;
    }

    try {
        auto replyFuture = _redisClient.hdel(key, fields);
        this->commit();
        if (!replyFuture.valid()) {
            LOG_ERROR("Future is not valid.");
            return false;
        }

        cpp_redis::reply reply = replyFuture.get();
        if (!reply) {
            LOG_ERROR("Cpp_redis reply is error or null.");
            return false;
        }

        if (!reply.is_integer() || (1 != reply.as_integer())) {
            LOG_ERROR("Reply return is not 1.");
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Cpp_redis exception: {}", e.what());
        return false;
    }

    return true;
}

