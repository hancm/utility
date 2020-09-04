#ifndef __ENCRYPT_UTILITY_H__
#define __ENCRYPT_UTILITY_H__

#include <string>
#include <vector>

#include <openssl/evp.h>
#include <openssl/pkcs7.h>

namespace encryptUtil
{

typedef struct
{
    X509 *x509;
    EVP_PKEY *pkey;
} openssl_x509_pkey;

/**
 * X509接口
 */

/**
 * @brief 从路径读取X509证书信息
 * @param [IN] x509Path         证书路径
 * @param [IN] format           证书格式: PEM、ASN1
 * @return X509*
 * 失败: NULL
 * @note
 */
X509 *LoadX509FromPath(const char *x509Path, const char *format = "PEM");

/**
 * @brief 从缓存读取X509证书信息
 * @param [IN] x509Buf         证书缓存
 * @param [IN] x509Len         证书缓存长度
 * @param [IN] format          证书格式: PEM、ASN1
 * @return X509*
 * 失败: NULL
 * @note
 */
X509 *LoadX509FromBuf(const char *x509Buf, size_t x509Len, const char *format = "PEM");

/**
 * @brief 释放x509证书
 * @param [IN] x509
 * @return void
 * @note
 */
void FreeX509(X509 *x509);

/**
 * @brief 读取X509证书的有效期
 * @param [IN] x509                  证书
 * @param [OUT] startTime            有效期开始时间(单位秒)
 * @param [OUT] endTime              有效期结束时间(单位秒)
 * @return int
 * @note
 */
int openssl_get_x509_valid_time(const X509 *x509, time_t &startTime, time_t &endTime);

/**
 * CRL接口
 */

/**
 * @brief 从路径读取crl信息
 * @param [IN] crlPath          crl路径
 * @param [IN] format           crl格式: PEM、ASN1
 * @return X509_CRL*
 * 失败: NULL
 * @note
 */
X509_CRL *LoadCRLFromPath(const char *crlPath, const char *format = "PEM");

/**
 * @brief 从缓存读取crl信息
 * @param [IN] crlBuf           crl缓存
 * @param [IN] crlLen           crl缓存长度
 * @param [IN] format           crl格式: PEM、ASN1
 * @return X509_CRL*
 * 失败: NULL
 * @note
 */
X509_CRL *LoadCRLFromBuf(const char *crlBuf, size_t crlLen, const char *format = "PEM");

/**
 * @brief 释放crl结构
 * @param [IN] crl          crl结构
 * @return void
 * @note
 */
void FreeX509CRL(X509_CRL *crl);

/**
 * @brief 判断用户证书是否被吊销
 * @param [IN] rootCert             根证书
 * @param [IN] Crl                  吊销列表
 * @param [IN] userCert             用户证书
 * @param [OUT] certValid           证书是否被吊销：true: 被吊销，false: 没有被吊销
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int CRL_Verify(X509 *rootCert, X509_CRL *Crl, X509 *userCert, bool &certValid);

/**
 * @brief 从路径读取信息判断证书是否被吊销
 * @param [IN] rootCertPath         根证书路径
 * @param [IN] crlPath              crl路径
 * @param [IN] userCertPath         用户证书路径
 * @param [OUT] certValid           证书是否被吊销：true: 被吊销，false: 没有被吊销
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int CRL_VerifyFromPath(const char *rootCertPath, const char *crlPath, const char *userCertPath, bool &certValid);

/**
 * PFX接口
 */

/**
 * @brief 从内存读取pfx信息
 * @param [IN] pfx_buf          pfx内存
 * @param [IN] pfx_len          pfx内存数据长度
 * @param [IN] password         pfx密码
 * @param [OUT] pfx             返回的证书、私钥
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int openssl_read_pfx_from_buf(const char *pfx_buf, size_t pfx_len, const char *password, openssl_x509_pkey &pfx);

/**
 * @brief 从路径读取pfx信息
 * @param [IN] pfile            pfx路径
 * @param [IN] password         pfx密码
 * @param [OUT] pfx             返回的证书、私钥
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int openssl_read_pfx_from_path(const char *pfile, const char *password, openssl_x509_pkey &pfx);

/**
 * @brief 从pfx路径或者缓存读取公私钥
 * @param [IN] pfxInfo      pfx路径或者缓存
 * @param [IN] pw           pfx密码
 * @param [OUT] pfx          pfx公私钥信息
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int openssl_read_pfx(const std::string &pfxInfo, const char *pw, openssl_x509_pkey &pfx);

/**
 * @brief 释放证书、私钥
 * @param [IN] pfx
 * @return void
 * @note
 */
void openssl_free_pfx(const openssl_x509_pkey &pfx);

/**
 * @brief 检查pfx中x509证书有效性, 包括有效期和是否吊销
 * @param [IN] pfxData          pfx证书缓存
 * @param [IN] password         pfx密码
 * @param [OUT] startTime       pfx中x509证书有效期开始时间
 * @param [OUT] endTime         pfx中x509证书有效期结束时间
 * @param [IN] rootCert         根证书
 * @param [IN] Crl              吊销列表
 * @param [OUT] certValid       pfx中x509证书是否被吊销：true: 被吊销，false: 没有被吊销
 * @return int
 * 成功: 0
 * 失败: -1, 证书无效(可能已过期或者被吊销); 若不检查证书是否被吊销则是证书已过期，否则可能证书过期或者被吊销
 * @note
 * rootCert != NULL && Crl != NULL && certValid != NULL: 此时才能获取是否被吊销
 */
int CheckPfxCertValid(const std::string &pfxData, const std::string &password,
                      time_t* const startTime = NULL, time_t* const endTime = NULL,
                      X509 *rootCert = NULL, X509_CRL *Crl = NULL, bool *certValid = NULL);

/**
 * Base64接口
 */

/**
 * @brief base64加密
 * @param [IN] src          待加密数据缓存
 * @param [IN] srcLen       待加密数据缓存长度
 * @return std::string
 * 成功: base64加密数据
 * @note
 */
std::string Base64Encode(const char *src, size_t srcLen);

/**
 * @brief base64数据解密
 * @param [IN] src              base64数据缓存
 * @param [IN] srcLen           base64数据缓存长度
 * @param [OUT] base64Decode    base64解密后的数据
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int Base64Decode(const char *src, size_t srcLen, std::string &base64Decode);

/**
 * SM4加解密
 */

/**
 * @brief 原数据加密，一次SM4_CBC加密---两次base64加密
 * @param [IN] inputSouce           待加密数据缓存
 * @param [IN] password             加密密码: SM3_KDF hash的前16字节作为SM4_CBC的iv、后16字节作为SM4_CBC的key
 * @param [OUT] encryptOutput       加密后的数据
 * @return int
 * 成功: 0
 * @note
 */
int SM4_DAO_encrypt(const std::string &inputSouce, const std::string &password, std::string &encryptOutput);

/**
 * @brief sm4加密后的数据解密，两次base64解密---一次SM4_CBC解密
 * @param [IN] inputEncrypt             sm4加密过的数据缓存
 * @param [IN] password                 解密密码：SM3_KDF hash的前16字节作为SM4_CBC的iv、后16字节作为SM4_CBC的key
 * @param [OUT] decryptOutput           解密后的数据
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int SM4_DAO_decrypt(const std::string &inputEncrypt, const std::string &password, std::string &decryptOutput);

/**
 * 证书解析
 */

/**
 * @brief 获取X509证书主题
 * @param [IN] x509                  证书
 * @param [OUT] x509NameSubject      证书主题,格式: E=zl03jsj@163.com, CN=user.zl, OU=zl, O=zl, ST=cq, C=cn
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetX509NameSubject(const X509 *x509, std::string &x509NameSubject);

/**
 * @brief 获取pfx中x509证书主题
 * @param [IN] pfx                      pfx证书缓存
 * @param [IN] pfxPasswd                pfx证书密码
 * @param [OUT] x509NameSubject         x509证书主题
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetPfxCertNameSubject(const std::string &pfx, const std::string &pfxPasswd, std::string &x509NameSubject);

/**
 * @brief 获取x509证书颁发机构
 * @param [IN] x509                 证书
 * @param [OUT] x509NameIssuer      证书颁发机构,格式："/C=CN/O=China Financial Certification Authority/CN=CFCA TEST OCA1"
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetX509NameIssuer(const X509 *x509, std::string &x509NameIssuer);

/**
 * @brief 获取pfx中x509证书颁发机构
 * @param [IN] pfx                  pfx证书缓存
 * @param [IN] pfxPasswd            pfx密码
 * @param [OUT] x509NameIssuer      x509证书颁发机构
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetPfxCertNameIssuer(const std::string &pfx, const std::string &pfxPasswd, std::string &x509NameIssuer);

std::string X509_GetSignatureOIDText(const X509 *x509);

/**
 * @brief 获取私钥的类型，rsa还是sm2
 * @param [IN] pkey             私钥
 * @return std::string
 * 成功: rsa、sm2
 * 失败: std::string()
 * @note
 */
std::string GetPKeyType(EVP_PKEY *pkey);

/**
 * @brief 获取pfx中私钥加密类型，rsa或者sm2
 * @param [IN] pfx                  pfx缓存
 * @param [IN] pfxPasswd            pfx密码
 * @param [OUT] pkeyType            加密类型: rsa、sm2
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int GetPfxPKeyType(const std::string &pfx, const std::string &pfxPasswd, std::string &pkeyType);

/**
 * pkcs7、CMS解析
 */

/**
 * @brief 转换pkcs7信息为普通文本
 * @param [IN] pkcs7            pkcs7结构
 * @param [OUT] p7Info          pkcs文本信息
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int PKCS7_get_info(PKCS7 *pkcs7, std::string &p7Info);

/**
 * @brief 获取pkcs7中签名信息的x509证书颁发机构
 * @param [IN] pkcs7            pkcs7文本
 * @param [OUT] issuer          颁发机构
 * @return int
 * 成功: 0
 * 失败: -1
 * @note
 */
int PKCS7_get_signatureinfo_issuer(const std::string &pkcs7, std::string &issuer);

} /* namespace encryptUtil */

#endif /* __ENCRYPT_UTILITY_H__ */
