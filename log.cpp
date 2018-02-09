#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <sys/stat.h>

#include "log.h"

using namespace MyUtilityLib;

static std::string g_logFile = "/tmp/zip_lib.log";
static std::fstream g_logStream(g_logFile, std::fstream::out | std::fstream::app);
static size_t g_logMaxSize = 10 * 1024 * 1024;

static int ReloadLogStream()
{
    struct stat fileInfo;
    if(stat(g_logFile.c_str(), &fileInfo) != 0) {
        return -1;
    }

    // 日志文件最大g_logMaxSize
    if((size_t)fileInfo.st_size >= g_logMaxSize) {
        g_logStream.clear();
        g_logStream.close();

        // 截断文件
        g_logStream.open(g_logFile, std::fstream::out | std::fstream::trunc);
        if (!g_logStream || !g_logStream.is_open()) {
            return -1;
        }

        return 0;
    }

    if (g_logStream && g_logStream.is_open()) {
        return 0;
    }

    g_logStream.clear();
    g_logStream.open(g_logFile, std::fstream::out | std::fstream::app);
    if (!g_logStream || !g_logStream.is_open()) {
        return -1;
    }

    return 0;
}

/**
 * Helper method for string formatting.
 *
 * @param fmt format of the string. Uses same formating as <code>printf()</code> function.
 * @param args parameters for the string format.
 * @return returns formatted string.
 * @see String::format(const char* fmt, ...)
 */
std::string Log::formatArgList(const char* fmt, va_list args)
{
    if(!fmt)
        return "";
    std::string result(2048, 0);
    int size = vsnprintf(&result[0], result.size() + 1, fmt, args);
    if(size == -1)
        return "";
    result.resize(size);
    return result;
}

void Log::out(LogType type, const char *date, const char *time, const char *file, unsigned int line, const char *func, const char *format, ...)
{
    if (0 != ReloadLogStream()) {
        return;
    }

    std::ostream *o = &g_logStream;
    *o << date << " " << time;

    switch(type)
    {
        case ErrorType: *o << "<ERROR>"; break;
        case WarnType: *o << "<WARN>"; break;
        case InfoType: *o << "<INFO>"; break;
        case DebugType: *o << "<DEBUG>"; break;
    }

    std::string strFile(file);
    size_t pos = strFile.find_last_of("/\\");
    std::string fileName = (pos == std::string::npos) ? strFile : strFile.substr(pos + 1);
    *o << "[" << fileName << ":" << line << "]<" << func << "> - ";

    va_list args;
    va_start(args, format);
    *o << formatArgList(format, args).c_str() << std::endl;
    va_end(args);
}

void Log::dbgPrintfMemImpl(const char *msg, const void *ptr, size_t size, const char *file, int line, const char *func, const char *date, const char *time)
{
    if (0 != ReloadLogStream()) {
        return;
    }

    std::ostream *o = &g_logStream;

    const unsigned char *data = (const unsigned char*)ptr;

    std::string strFile(file);
    size_t pos = strFile.find_last_of("/\\");
    std::string fileName = (pos == std::string::npos) ? strFile : strFile.substr(pos + 1);
    *o << date << " " << time << "<DEBUG>[" << fileName << ":" << line << "]<" << func << "> - " << msg << " { ";
    *o << std::hex << std::uppercase << std::setfill('0');
    for (size_t i = 0; i < size; ++i) {
        *o << std::setw(2) << static_cast<int>(data[i]) << ' ';
    }
    *o << std::dec << std::nouppercase << std::setfill(' ') <<"}:" << size << std::endl;
}
