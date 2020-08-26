#ifndef __HTTP_PARSE_UTILITY__
#define __HTTP_PARSE_UTILITY__

#include <vector>
#include <string>
#include <map>

#include "./http-parser/http_parser.h"

namespace util
{

class HttpParser
{
public:
    HttpParser();

    int parseRequest(const char *requestData, int len);
    int parseResponse(const char *responseData, int len);

    /**
     * 请求信息
     */
    bool hasHttpRequest = false;
    std::string method;
    std::string url;
    std::string version;

    // <field, value>
    std::map<std::string, std::string> requestHeaderMap;
    std::string requestBody;

    /**
     * 响应信息
     */
    bool hasHttpResponse = false;
    int statusCode = 0;

    // <field, value>
    std::map<std::string, std::string> responseHeaderMap;
    std::string responseBody;

private:
    http_parser _request_parser;
    http_parser _response_parser;
};

} /* namespace util */
#endif /* __HTTP_PARSE_UTILITY__ */