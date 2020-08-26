#ifndef __HTTP_CLIENT_UTILITY__
#define __HTTP_CLIENT_UTILITY__

#include <vector>
#include <string>

namespace util
{

namespace http_client
{

#define CONTENT_TYPE_X_WWW_FORM_URLENCODED      "Content-Type: application/x-www-form-urlencoded"
#define CONTENT_TYPE_TIMESTAMP_QUERY            "Content-Type: application/timestamp-query"

/**
 * @brief http客户端请求，默认使用get方法，可选添加http head、post请求body数据
 *        通过添加postFiledData信息既为使用post方式，不可以没有body的post请求
 * @param [IN] url                  http url
 * @param [IN] headerList           自定义的http头, empty: 默认头
 * @param [IN] postFiledData        post方式body数据缓存，NULL: 使用get方式
 * @param [IN] postFiledDataLen     post方式body数据缓存长度
 * @param [IN] responseData         响应数据
 * @return int
 * @note
 */
int HttpRequestWithPostAndHeader(const char *url, const std::vector<std::string> &headerList,
                                 const char *postFiledData, int postFiledDataLen,
                                 std::string &responseData);

} /* namespace http_client */

} /* namespace util */
#endif /* __HTTP_CLIENT_UTILITY__ */