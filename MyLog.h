#ifndef __MYLOG_H__
#define __MYLOG_H__

// Usage:
// 定义日志路径：static MyLog MLog("/tmp/MyLog.log");
// LOG_TRACE(); LOG_INFO(); 打印日志
// 编译release、链接时优化选项增加速度：-W -pthread -O3 -flto -DNDEBUG

// 0：rotating日志
// 1：rotating日志 + 增加控制台日志
#define _MYLOG_SINK_    0

// 0: trace级别
// 1：ERROR级别
#define _MYLOG_LEVEL_   0

// 在spdlog.h之前定义才有效
// SPDLOG_TRACE调试宏
#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif

// SPDLOG_DEBUG调试宏
#ifndef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#endif

// Linux使用CLOCK_REALTIME_COARSE
#ifndef SPDLOG_CLOCK_COARSE
#define SPDLOG_CLOCK_COARSE
#endif

// 引入log头文件
#include "spdlog/spdlog.h"

#define MY_LOG_NAME ("MyLog")

// spdlog封装
class MyLog
{
public:
    MyLog(const std::string &file_name) noexcept
    {
        destroy();
        init(file_name);
    }

    template <typename... Args>
    static inline void trace(const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger();
            if (logger) {
                logger->trace(format, args...);
            }
        } catch (...) {
            destroy();
        }
    }

    template <typename... Args>
    static inline void debug(const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger();
            if (logger) {
                logger->debug(format, args...);
            }
        } catch (...) {
            destroy();
        }
    }

    template <typename... Args>
    static inline void info(const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger();
            if (logger) {
                logger->info(format, args...);
            }
        } catch (...) {
            destroy();
        }
    }

    template <typename... Args>
    static inline void warn(const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger();
            if (logger) {
                logger->warn(format, args...);
            }
        } catch (...) {
            destroy();
        }
    }

    template <typename... Args>
    static inline void error(const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger();
            if (logger) {
                logger->error(format, args...);
            }
        } catch (...) {
            destroy();
        }
    }

    template <typename... Args>
    static inline void critical(const char *format, const Args&... args) noexcept
    {
        try {
            std::shared_ptr<spdlog::logger> logger = getLogger();
            if (logger) {
                logger->critical(format, args...);
            }
        } catch (...) {
            destroy();
        }
    }

private:
    MyLog(const MyLog&) = delete;
    MyLog& operator=(const MyLog&) = delete;

private:
    static inline void init(const std::string &file_name) noexcept
    {
        try {
            if (file_name.empty() || getLogger()) {
                return;
            }

            std::vector<spdlog::sink_ptr> sinks;

#if (1 == _MYLOG_SINK_)
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
#endif

            // rotating日志MyLog, 10M * 10个备份
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_name, 1024 * 1024 * 10, 10));

            std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(MY_LOG_NAME, begin(sinks), end(sinks));

            // register日志，这是一个单例
            spdlog::register_logger(logger);

            // 设置日志级别
#if (0 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::trace);
#elif (1 == _MYLOG_LEVEL_)
            logger->set_level(spdlog::level::err);
#endif

            //设置日志格式[年-月-日 时:分:秒.毫秒][进程号:线程号][调试级别](宏定义中添加: [文件名:行号]<函数名> - 具体内容)
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%P:%t][%l]%v");

            //设置当出发err或更严重的错误时立刻刷新日志到disk
            logger->flush_on(spdlog::level::err);
        } catch (...) {
            destroy();
        }
    };

    static inline void destroy() noexcept
    {
        try {
            spdlog::drop_all();
        } catch (...) {
            // NULL
        }
    }

    static inline std::shared_ptr<spdlog::logger> getLogger() noexcept
    {
        try {
            return spdlog::get(MY_LOG_NAME);
        } catch (...) {
            return nullptr;
        }
    }
};

// 整数类型文件行号
#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

// 格式化
#ifndef FORMATTED
#define FORMATTED(format)   (std::string("[").append(__FILENAME__).append(":").append(std::to_string(__LINE__)) \
                             .append("]<").append(__FUNCTION__).append("> - ").append(format).c_str())
#endif

#define LOG_TRACE(format, ...)      (MyLog::trace(FORMATTED(format), ##__VA_ARGS__))
#define LOG_DEBUG(format, ...)      (MyLog::debug(FORMATTED(format), ##__VA_ARGS__))
#define LOG_INFO(format, ...)      (MyLog::info(FORMATTED(format), ##__VA_ARGS__))
#define LOG_WARN(format,...)      (MyLog::warn(FORMATTED(format), ##__VA_ARGS__))
#define LOG_ERROR(format,...)     (MyLog::error(FORMATTED(format), ##__VA_ARGS__))
#define LOG_SYS(format,...)       (MyLog::critical(FORMATTED(format), ##__VA_ARGS__))

#endif /* __MYLOG_H__ */
