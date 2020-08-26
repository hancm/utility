#include <string.h>

#include <string>

#include "MyLog.h"
#include "./http-parser/http_parser.h"
#include "http_parse_utility.h"

namespace util
{

static http_parser_settings g_request_settings;
static http_parser_settings g_response_settings;

/**
 * 请求回调函数
 */
static int on_url(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->url.append(at, length);
    return 0;
}

static int on_request_header_field(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->requestHeaderMap[std::string(at, length)] = "";
    return 0;
}

static int on_request_header_value(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    for (auto &pair: httpParseUtil->requestHeaderMap)
    {
        if (pair.second.empty())
        {
            pair.second.assign(at, length);
        }
    }

    return 0;
}

static int on_request_headers_complete(http_parser *_)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->method = http_method_str((enum http_method)_->method);
    httpParseUtil->version = std::to_string(_->http_major) + "." + std::to_string(_->http_minor);
    return 0;
}

static int on_request_body(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->requestBody.append(at, length);
    return 0;
}

/**
 * 响应回调函数
 */
static int on_status(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->statusCode = _->status_code;
    return 0;
}

static int on_response_header_field(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->responseHeaderMap[std::string(at, length)] = "";
    return 0;
}

static int on_response_header_value(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    for (auto &pair: httpParseUtil->responseHeaderMap)
    {
        if (pair.second.empty())
        {
            pair.second.assign(at, length);
        }
    }
    return 0;
}

static int on_response_body(http_parser *_, const char *at, size_t length)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    httpParseUtil->responseBody.append(at, length);
    return 0;
}

static int session_over(http_parser *_)
{
    HttpParser *httpParseUtil = (HttpParser*)_->data;
    (void)httpParseUtil;
    return 0;
}

class InitSetting
{
public:
    InitSetting()
    {
        /**
         * 请求设置
         */
        memset(&g_request_settings, 0, sizeof(g_request_settings));
        g_request_settings.on_url = on_url;
        g_request_settings.on_header_field = on_request_header_field;
        g_request_settings.on_header_value = on_request_header_value;
        g_request_settings.on_headers_complete = on_request_headers_complete;
        g_request_settings.on_body = on_request_body;

        /**
         * 响应设置
         */
        memset(&g_response_settings, 0, sizeof(g_response_settings));
        g_response_settings.on_status = on_status;
        g_response_settings.on_header_field = on_response_header_field;
        g_response_settings.on_header_value = on_response_header_value;
        g_response_settings.on_body = on_response_body;
        g_response_settings.on_message_complete = session_over;
    }
};

HttpParser::HttpParser()
{
    static InitSetting initSetting;

    _request_parser.data = this;
    http_parser_init(&_request_parser, HTTP_REQUEST);

    _response_parser.data = this;
    http_parser_init(&_response_parser, HTTP_RESPONSE);
}

int HttpParser::parseRequest(const char *requestData, int len)
{
    int nRet = 0;
    if (NULL == requestData || 0 >= len)
    {
        LOG_ERROR("Empty request data.");
        return -1;
    }

    nRet = http_parser_execute(&_request_parser, &g_request_settings, requestData, len);
    if (nRet != len)
    {
        LOG_ERROR("Failed to http parse execute request data.");
        return -1;
    }

    hasHttpRequest = true;
    return 0;
}

int HttpParser::parseResponse(const char *responseData, int len)
{
    int nRet = 0;
    if (NULL == responseData || 0 >= len)
    {
        LOG_ERROR("Empty response data.");
        return -1;
    }

    nRet = http_parser_execute(&_response_parser, &g_response_settings, responseData, len);
    if (nRet != len)
    {
        LOG_ERROR("Failed to http parse execute response data.");
        return -1;
    }

    hasHttpResponse = true;
    return 0;
}

} /* namespace util */
