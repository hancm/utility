#include <string>

#include "CivetServer.h"
#include "MyLog.h"

#include "http_service.h"
#include "http_controller.h"
#include "http_error.h"

#include "json_utility.h"

#define HTTP_ERROR_FORMAT           ("{\n" \
                                     "    \"code\": \"%d\",\n" \
                                     "    \"message\": \"%s\"\n" \
                                    "}")

#define HTTP_POST_RETURN_FORMAT     ("{\n" \
                                     "    \"code\": \"%d\",\n" \
                                     "    \"message\": \"%s\",\n" \
                                     "    \"data\": %s\n" \
                                    "}")

namespace controller
{

static void sendHttpTextChunk(struct mg_connection *conn, const std::string &message)
{
    mg_send_http_ok(conn, "text/plain", -1);
    mg_send_chunk(conn, &message[0], message.size());
    mg_send_chunk(conn, 0, 0);
}

static void sendHTTPError(struct mg_connection *conn, int errorNum)
{
    std::string errMessage("未知的错误");
    if (HTTP_ERROR_START_VALUE < errorNum && HTTP_ERROR_MAX_VALUE > errorNum) {
        // http错误码
        auto iter = g_httpErrnoStringMap.find(errorNum);
        if (g_httpErrnoStringMap.end() != iter) {
            errMessage = iter->second;
        }
    }

    std::string errString(1024, '\0');
    snprintf(&errString[0], errString.size(), HTTP_ERROR_FORMAT, errorNum, errMessage.c_str());
    errString.resize(strlen(&errString[0]));
    errString.shrink_to_fit();
    LOG_DEBUG("Http error message: [{}]", errString);
    sendHttpTextChunk(conn, errString);
    return;
}

static void sendHttpSucceed(struct mg_connection *conn)
{
    char response[200] = {0};
    snprintf(response, sizeof(response), HTTP_ERROR_FORMAT, HTTP_ERROR_START_VALUE, "Succeed");
    sendHttpTextChunk(conn, response);
    return;
}

static void sendHttpResponse(struct mg_connection *conn, const std::string &dataJson)
{
    std::string responseJson(dataJson.size() + 100, '\0');
    snprintf(&responseJson[0], responseJson.size(), HTTP_POST_RETURN_FORMAT, HTTP_ERROR_START_VALUE, "Succeed", dataJson.c_str());
    responseJson.resize(strlen(&responseJson[0]));
    responseJson.shrink_to_fit();

    LOG_DEBUG("Http response: [{}]", responseJson);
    sendHttpTextChunk(conn, responseJson);
    return;
}

static void sendHttpResponse(struct mg_connection *conn, const std::map<std::string, std::string> &keyValueMap)
{
    util::ordered_json json(keyValueMap);
    sendHttpResponse(conn, json.dump());
    return;
}

bool HelloHandler::handlePost(CivetServer *server, struct mg_connection *conn)
{
    int nRet = 0;

    // http请求URL
    const mg_request_info *requestInfo = mg_get_request_info(conn);
    std::string requestUrl = requestInfo->request_uri;
    LOG_DEBUG("Request URL: [{}]", requestUrl);

#if 0
    // 获取POST BODY数据
    std::string postData = server->getPostData(conn);
    LOG_DEBUG("Hello post data: [{}]", postData);
#endif

    /**
     * 获取x-www-form-urlencoded表单数据
     * key1=value1&key2=value2&key3=value3
     * 根据key获取value值
     */
    std::string echo;
    if (!server->getParam(conn, "echo", echo)) {
        LOG_ERROR("Failed to find param.");
        nRet= ERR_PARAM_INVALID;
    }

    if (0 != nRet) {
        sendHTTPError(conn, nRet);
        return true;
    }

    sendHttpResponse(conn, std::map<std::string, std::string>{{"echo", echo}});
    return true;
}

} /* namespace controller */