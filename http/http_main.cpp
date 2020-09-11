#include <iostream>

#include <getopt.h>

#include "CivetServer.h"
#include "MyLog.h"

#include "http_env.h"
#include "http_controller.h"

// 版本信息
const char *verbose = "1.0.0";

bool exitNow = false;

// 段选项参数
//char*optstring = “ab:c::”;
//单个字符a         表示选项a没有参数            格式：-a即可，不加参数
//单字符加冒号b:     表示选项b有且必须加参数      格式：-b 100或-b100,但-b=100错
//单字符加2冒号c::   表示选项c可以有，也可以无     格式：-c200，其它格式错误
const char *main_optstring = "f:hv";

// 长选项参数
// 3参数NULL, 4参数作为匹配长选项的返回值用于switch参数判断
const struct option main_longopts[] = {
    {"config",  required_argument, NULL, 'f'},
    {"help",    no_argument,       NULL, 'h'},
    {"version", no_argument,       NULL, 'v'}
};

static void usage()
{
    std::cout << "Usage: http -f <config_path> [-h] [-v]" << std::endl;
    std::cout << "-f --config config file path." << std::endl;
    std::cout << "-h --help help info." << std::endl;
    std::cout << "-v --version version info." << std::endl;
}

static int http_error_log(const struct mg_connection *conn, const char *message)
{
    LOG_ERROR(message);
    return 1;
}

static void printHttpStatus(const struct mg_context *ctx)
{
    std::vector<char> buffer(1024);
    int contextLen = mg_get_context_info(ctx, &buffer[0], buffer.size());
    buffer[contextLen] = '\0';
    MYLOG_TRACE(MYLOG_NAME_HTTPSTATUS, "Http context info: [{}]", &buffer[0]);
    return;
}

int main(int argc, char *argv[])
{
    int nRet = 0;

    /**
     * 读取命令行参数
     */
    std::string confPath;
    int opt = 0;
    while ((opt = getopt_long(argc, argv, main_optstring, main_longopts, NULL)) != -1) {
        switch ((opt))
        {
            case 'f':
            {
                confPath = optarg;
                break;
            }
            case 'h':
            {
                usage();
                return 0;
            }
            case 'v':
            {
                std::cout << verbose << std::endl;
                return 0;
            }
            default:
            {
                usage();
                return -1;
            }
        }
    }

    if (confPath.empty()) {
        usage();
        return -1;
    }
    LOG_DEBUG("XML conf path: {}", confPath);

    /**
     * 初始化日志
     */
    nRet = env::InitLog(confPath);
    if (0 != nRet) {
        LOG_ERROR("Failed to init log.");
        return nRet;
    }

    /**
     * 初始化配置文件
     */
    nRet = env::InitConfig(confPath, env::g_config);
    if (0 != nRet) {
        LOG_ERROR("Failed to init config.");
        return nRet;
    }

    /**
     * 启动civetserver HTTP服务
     */
    CivetCallbacks callbacks;
    callbacks.log_message = http_error_log;
    CivetServer server(env::g_config.httpConf, &callbacks);

    // hello回显测试程序
    controller::HelloHandler helloHandle;
    server.addHandler(HTTP_HELLO_URL, helloHandle);

    /**
     * 主线程休眠
     */
    LOG_DEBUG("##############################HTTP SERVER START##############################");
    while (!exitNow)
    {
        // 休眠1秒
        sleep(1);

        // 打印http状态信息
        if ("1" == env::g_config.httpStatus) {
            printHttpStatus(server.getContext());
        }
    }

    return 0;
}
