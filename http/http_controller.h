#ifndef __HTTP_CONTROLLER_H__
#define __HTTP_CONTROLLER_H__

#include <string>
#include <vector>
#include <map>

#include "CivetServer.h"

namespace controller
{

/**
 * hello回显post测试函数
 * 参数: echo="回显字符串"
 */
// URL路径
#define HTTP_HELLO_URL      "/hello"
class HelloHandler : public CivetHandler
{
public:
    bool handlePost(CivetServer *server, struct mg_connection *conn);
};

} /* namespace controller */

#endif /* __HTTP_CONTROLLER_H__ */
