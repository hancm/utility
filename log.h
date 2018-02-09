#pragma once

#include <string>
#include <cstdarg>

namespace MyUtilityLib
{
    class Log
    {
    public:
        enum LogType
        {
            ErrorType = 0,
            WarnType,
            InfoType,
            DebugType
        };

        static void out(LogType type, const char *date, const char *time, const char *file, unsigned int line, const char *func, const char *format, ...);
        static void dbgPrintfMemImpl(const char *msg, const void *ptr, size_t size, const char *file, int line, const char *func, const char *date, const char *time);
    private:
        static std::string formatArgList(const char *fmt, va_list args);
    };
}

#define ERROR(...)  MyUtilityLib::Log::out(MyUtilityLib::Log::ErrorType, __DATE__, __TIME__, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define WARN(...)   MyUtilityLib::Log::out(MyUtilityLib::Log::WarnType, __DATE__, __TIME__, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define INFO(...)   MyUtilityLib::Log::out(MyUtilityLib::Log::InfoType, __DATE__, __TIME__, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define DEBUG(...)  MyUtilityLib::Log::out(MyUtilityLib::Log::DebugType, __DATE__, __TIME__, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define DEBUGMEM(msg, ptr, size) MyUtilityLib::Log::dbgPrintfMemImpl(msg, ptr, size, __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)
