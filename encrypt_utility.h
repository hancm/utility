#ifndef __ENCRYPT_UTILITY_H__
#define __ENCRYPT_UTILITY_H__

#include <string>
#include <vector>

#include <openssl/evp.h>

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
X509 *LoadX509FromPath(const char *x509Path, const char *format = "PEM");
X509 *LoadX509FromBuf(const char *x509Buf, size_t x509Len, const char *format = "PEM");
void FreeX509(X509 *x509);

int openssl_get_x509_valid_time(const X509 *x509, time_t &startTime, time_t &endTime);

/**
 * CRL接口
 */
X509_CRL *LoadCRLFromPath(const char *crlPath, const char *format = "PEM");
X509_CRL *LoadCRLFromBuf(const char *crlBuf, size_t crlLen, const char *format = "PEM");
void FreeX509CRL(X509_CRL *crl);

int CRL_Verify(X509 *rootCert, X509_CRL *Crl, X509 *userCert, bool &certValid);
int CRL_VerifyFromPath(const char *rootCertPath, const char *crlPath, const char *userCertPath, bool &certValid);

/**
 * PFX接口
 */
int openssl_read_pfx_from_buf(const char *pfx_buf, const size_t pfx_len, const char *password, openssl_x509_pkey &pfx);
int openssl_read_pfx_from_path(const char *pfile, const char *password, openssl_x509_pkey &pfx);
void openssl_free_pfx(const openssl_x509_pkey &pfx);

/**
 * @brief 检查pfx中证书有效性包括有效期和是否吊销
 * @param [IN] pfxData
 * @param [IN] password
 * @return int
 * @note
 */
int CheckPfxCertValid(const std::string &pfxData, const std::string &password,
                      time_t* const startTime = NULL, time_t* const endTime = NULL,
                      X509 *rootCert = NULL, X509_CRL *Crl = NULL, bool *certValid = NULL);

/**
 * Base64接口
 */
std::string Base64Encode(const char *src, size_t srcLen);
int Base64Decode(const char *src, size_t srcLen, std::string &base64Decode);

/**
 * 数据库参数解密
 */
int SM4_DAO_decrypt(const std::string &inputEncrypt, const std::string &password, std::string &decryptOutput);
int SM4_DAO_encrypt(const std::string &inputSouce,
                    const std::string &password,
                    std::string &encryptOutput);

/**
 * 证书解析
 */
int GetX509NameSubject(const X509 *x509, std::string &x509NameSubject);
int GetPfxCertNameSubject(const std::string &pfx,
                          const std::string &pfxPasswd,
                          std::string &x509NameSubject);

int GetX509NameIssuer(const X509 *x509, std::string &x509NameIssuer);
int GetPfxCertNameIssuer(const std::string &pfx,
                         const std::string &pfxPasswd,
                         std::string &x509NameIssuer);

std::string X509_GetSignatureOIDText(const X509 *x509);

std::string GetPKeyType(EVP_PKEY *pkey);
int GetPfxPKeyType(const std::string &pfx,
                   const std::string &pfxPasswd,
                   std::string &pkeyType);

} /* namespace encrypt */

#endif /* __ENCRYPT_UTILITY_H__ */
