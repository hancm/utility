#ifndef __MYLOG_H__
#define __MYLOG_H__

// Usage:
// 定义日志路径：static util::MyLog MLog("/var/log/MyLog.log");
// LOG_DEBUG(); LOG_INFO(); LOG_ERROR; 打印日志
// 编译release、链接时优化选项增加速度：-Wall -pthread -O3 -flto -DNDEBUG

// 0：rotating日志
// 1：rotating日志 + 增加控制台日志
#ifndef _MYLOG_SINK_
#define _MYLOG_SINK_    1
#endif

// 0: TRACE
// 1: DEBUG
// 2: INFO
// 3: WARN
// 4: ERROR
#ifndef _MYLOG_LEVEL_
#define _MYLOG_LEVEL_   0
#endif

//
// SPDLOG宏定义必须在spdlog.h之前定义才有效
//

// 日志级别名称
#ifndef SPDLOG_LEVEL_NAMES
#define SPDLOG_LEVEL_NAMES { "TRACE", "DEBUG", "INFO",  "WARNING", "ERROR", "CRITICAL", "OFF" }
#endif

// 引入log头文件
#include "spdlog/spdlog.h"
//#include "spdlog/cfg/env.h"                         // for loading levels from the environment variable
//#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace util
{

#define MY_LOG_NAME ("__MY_LOG_NAME__")

// spdlog封装
class MyLog
{
public:
    MyLog() {};

    MyLog(const std::string &file_name, const char *logName = MY_LOG_NAME) noexcept
    {
        _init(file_name, logName);
    }

    static inline void init(const std::string &file_name, const char *logName = MY_LOG_NAME) noexcept
    {
        _init(file_name, logName);
    }

    static inline void destory() noexcept
    {
        _destroy();
    }

    template <typename... Args>
    static inline void trace(const char *logName, const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger(logName);
            if (logger) {
                logger->trace(format, args...);
            }
        } catch (...) {
            _destroy();
        }
    }

    template <typename... Args>
    static inline void debug(const char *logName, const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger(logName);
            if (logger) {
                logger->debug(format, args...);
            }
        } catch (...) {
            _destroy();
        }
    }

    template <typename... Args>
    static inline void info(const char *logName, const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger(logName);
            if (logger) {
                logger->info(format, args...);
            }
        } catch (...) {
            _destroy();
        }
    }

    template <typename... Args>
    static inline void warn(const char *logName, const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger(logName);
            if (logger) {
                logger->warn(format, args...);
            }
        } catch (...) {
            _destroy();
        }
    }

    template <typename... Args>
    static inline void error(const char *logName, const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger(logName);
            if (logger) {
                logger->error(format, args...);
            }
        } catch (...) {
            _destroy();
        }
    }

    template <typename... Args>
    static inline void critical(const char *logName, const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger(logName);
            if (logger) {
                logger->critical(format, args...);
            }
        } catch (...) {
            _destroy();
        }
    }

private:
    MyLog(const MyLog&) = delete;
    MyLog& operator=(const MyLog&) = delete;

private:
    static inline void _init(const std::string &file_name, const char *logName) noexcept
    {
        try {
            if (file_name.empty() || getLogger(logName)) {
                return;
            }

            // 设置异步日志模式
//          spdlog::set_async_mode(4096);

            // 添加目的端
            std::vector<spdlog::sink_ptr> sinks;

            // 标准输出
#if (1 == _MYLOG_SINK_)
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
#endif

            // rotating日志MyLog, 10M * 10个备份
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_name, 1024 * 1024 * 10, 10));

            std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(logName, begin(sinks), end(sinks));

            // register日志，这是一个单例
            spdlog::register_logger(logger);

            // 设置日志级别
#if (0 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::trace);
#elif (1 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::debug);
#elif (2 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::info);
#elif (3 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::warn);
#elif (4 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::err);
#endif

            //设置日志格式[年-月-日 时:分:秒.毫秒][进程号:线程号][调试级别](宏定义中添加: [文件名:行号]<函数名> - 具体内容)
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%P:%t][%l]%v");

            //设置当出发debug或更严重的错误时立刻刷新日志到disk
            logger->flush_on(spdlog::level::debug);
        } catch (...) {
            _destroy();
        }
    };

    static inline void _destroy() noexcept
    {
        try {
            // Release all spdlog resources, and drop all loggers in the registry.
            // This is optional (only mandatory if using windows + async log).
            spdlog::shutdown();
        } catch (...) {
            // NULL
        }
    }

    static inline std::shared_ptr<spdlog::logger> getLogger(const char *logName) noexcept
    {
        try {
            return spdlog::get(logName);
        } catch (...) {
            return nullptr;
        }
    }
};

} /* namespace util */

// 整数类型文件行号
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

// 格式化
#ifndef FORMATTED
#define FORMATTED(format)    "[{:s}:{:d}]<{:s}> - " #format, __FILE__, __LINE__, __FUNCTION__
#endif

#define MYLOG_TRACE(__MYLOG_NAME__, format, ...)      (util::MyLog::trace(__MYLOG_NAME__, FORMATTED(format), ##__VA_ARGS__))
#define MYLOG_DEBUG(__MYLOG_NAME__, format, ...)      (util::MyLog::debug(__MYLOG_NAME__, FORMATTED(format), ##__VA_ARGS__))
#define MYLOG_INFO(__MYLOG_NAME__, format, ...)       (util::MyLog::info(__MYLOG_NAME__, FORMATTED(format), ##__VA_ARGS__))
#define MYLOG_WARN(__MYLOG_NAME__, format, ...)        (util::MyLog::warn(__MYLOG_NAME__, FORMATTED(format), ##__VA_ARGS__))
#define MYLOG_ERROR(__MYLOG_NAME__, format, ...)       (util::MyLog::error(__MYLOG_NAME__, FORMATTED(format), ##__VA_ARGS__))
#define MYLOG_SYS(__MYLOG_NAME__, format, ...)         (util::MyLog::critical(__MYLOG_NAME__, FORMATTED(format), ##__VA_ARGS__))

// 默认的日志: MyLog
#define LOG_TRACE(format, ...)      (util::MyLog::trace(MY_LOG_NAME, FORMATTED(format), ##__VA_ARGS__))
#define LOG_DEBUG(format, ...)      (util::MyLog::debug(MY_LOG_NAME, FORMATTED(format), ##__VA_ARGS__))
#define LOG_INFO(format, ...)      (util::MyLog::info(MY_LOG_NAME, FORMATTED(format), ##__VA_ARGS__))
#define LOG_WARN(format, ...)      (util::MyLog::warn(MY_LOG_NAME, FORMATTED(format), ##__VA_ARGS__))
#define LOG_ERROR(format, ...)     (util::MyLog::error(MY_LOG_NAME, FORMATTED(format), ##__VA_ARGS__))
#define LOG_SYS(format, ...)       (util::MyLog::critical(MY_LOG_NAME, FORMATTED(format), ##__VA_ARGS__))

#endif /* __MYLOG_H__ */
