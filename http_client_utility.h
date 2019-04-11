#ifndef __HTTP_CLIENT_UTILITY__
#define __HTTP_CLIENT_UTILITY__

namespace HttpClientUtility
{

#include <string>

/**
 * @brief http客户端请求，默认使用get方法，可选添加http head、post请求body数据
 *        通过添加postFiledData信息既为使用post方式，不可以没有body的post请求
 * @param [IN] url                  http url
 * @param [IN] customHeader         自定义的http头, NULL: 默认头
 * @param [IN] postFiledData        post方式body数据缓存，NULL: 使用get方式
 * @param [IN] postFiledDataLen     post方式body数据缓存长度
 * @param [IN] responseData         响应数据
 * @return int
 * @note
 */
int HttpRequestWithPostAndHeader(const char *url, const char *customHeader,
                                 const char *postFiledData, int postFiledDataLen,
                                 std::string &responseData);

} /* namespace HttpClient */
#endif /* __HTTP_CLIENT_UTILITY__ */