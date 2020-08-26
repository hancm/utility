#include <string>
//#include <sstream>
//#include <map>
//#include<fstream>
//
//#include <openssl/evp.h>
//#include <openssl/engine.h>
//#include <openssl/crypto.h>
//#include <openssl/pem.h>
//#include <openssl/err.h>
//#include <openssl/x509_vfy.h>
//#include <openssl/x509v3.h>
//#include <openssl/cms.h>
//#include <openssl/bio.h>
//
//#include <openssl/ssl.h>
//#include "openssl/err.h"
//#include "openssl/bio.h"
//#include "openssl/asn1.h"
//#include "openssl/x509.h"
//#include "openssl/x509v3.h"
//#include "openssl/err.h"
//#include "openssl/objects.h"
//#include "openssl/pem.h"
//#include "openssl/pkcs7.h"
//#include "openssl/pkcs12.h"
//#include "openssl/opensslv.h"
//#include "openssl/ts.h"
//
//#include "sm3.h"
//#include "sm4.h"

#include "MyLog.h"
#include "encrypt_utility.h"

namespace util
{

//static bool fileExists(const std::string &path)
//{
//    struct stat fileInfo;
//    if(stat(path.c_str(), &fileInfo) != 0)
//        return false;
//
//    // XXX: != S_IFREG
//    return !((fileInfo.st_mode & S_IFMT) == S_IFDIR);
//}
//
//class OpensslInit
//{
//public:
//    OpensslInit()
//    {
//        // digests and ciphers
//        // 退出时EVP_cleanup进行清理
//        OpenSSL_add_all_algorithms();
//        SSL_library_init();
//
//        SSL_load_error_strings();
//        ERR_load_ERR_strings();
//        ERR_load_crypto_strings();
//        ERR_load_BIO_strings();
//        ERR_load_TS_strings();
//
//        ERR_clear_error();
//    }
//
//    ~OpensslInit()
//    {
//        // thread-safe cleanup
//        ENGINE_cleanup();
//        CONF_modules_unload(1);
//
//        // global application exit cleanup (after all SSL activity is shutdown)
//        ERR_free_strings();
//        CRYPTO_cleanup_all_ex_data();
//        OBJ_cleanup();
//        EVP_cleanup();
//    }
//};
//
//// 初始化openssl
//static OpensslInit opensslInit;;
//
//static std::string opensslGetErrorString()
//{
//    unsigned long ulErr = ERR_get_error();
//
//    char szErrMsg[1024] = {0};
//    ERR_error_string(ulErr, szErrMsg); // 格式：error:errId:库:函数:原因
//    return szErrMsg;
//}

static void base64_encode(const unsigned char *src, int src_len, char *dst)
{
    static const char *b64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j, a, b, c;

    for (i = j = 0; i < src_len; i += 3) {
        a = src[i];
        b = ((i + 1) >= src_len) ? 0 : src[i + 1];
        c = ((i + 2) >= src_len) ? 0 : src[i + 2];

        dst[j++] = b64[a >> 2];
        dst[j++] = b64[((a & 3) << 4) | (b >> 4)];
        if (i + 1 < src_len) {
            dst[j++] = b64[(b & 15) << 2 | (c >> 6)];
        }
        if (i + 2 < src_len) {
            dst[j++] = b64[c & 63];
        }
    }
    while (j % 4 != 0) {
        dst[j++] = '=';
    }
    dst[j++] = '\0';
}

std::string Base64Encode(const char *src, size_t srcLen)
{
    std::string encode;
    encode.resize(2 * srcLen);
    base64_encode((const unsigned char*)src, srcLen, &encode[0]);
    encode.resize(strlen(&encode[0]));
    encode.shrink_to_fit();

    return encode;
}

static unsigned char b64reverse(char letter)
{
    if ((letter >= 'A') && (letter <= 'Z')) {
        return letter - 'A';
    }
    if ((letter >= 'a') && (letter <= 'z')) {
        return letter - 'a' + 26;
    }
    if ((letter >= '0') && (letter <= '9')) {
        return letter - '0' + 52;
    }
    if (letter == '+') {
        return 62;
    }
    if (letter == '/') {
        return 63;
    }
    if (letter == '=') {
        return 255; /* normal end */
    }
    return 254; /* error */
}

static int base64_decode(const unsigned char *src, int src_len, char *dst, size_t *dst_len)
{
    int i;
    unsigned char a, b, c, d;

    *dst_len = 0;

    for (i = 0; i < src_len; i += 4) {
        a = b64reverse(src[i]);
        if (a >= 254) {
            return i;
        }

        b = b64reverse(((i + 1) >= src_len) ? 0 : src[i + 1]);
        if (b >= 254) {
            return i + 1;
        }

        c = b64reverse(((i + 2) >= src_len) ? 0 : src[i + 2]);
        if (c == 254) {
            return i + 2;
        }

        d = b64reverse(((i + 3) >= src_len) ? 0 : src[i + 3]);
        if (d == 254) {
            return i + 3;
        }

        dst[(*dst_len)++] = (a << 2) + (b >> 4);
        if (c != 255) {
            dst[(*dst_len)++] = (b << 4) + (c >> 2);
            if (d != 255) {
                dst[(*dst_len)++] = (c << 6) + d;
            }
        }
    }

    // 返回已经解码的字符数
    // 成功: i应该等于src_len
    return i;
}

int Base64Decode(const char *src, size_t srcLen, std::string &base64Decode)
{
    if (NULL == src || 0 >= srcLen) {
        LOG_ERROR("Null src or src len <= 0.");
        return -1;
    }

    if (0 != srcLen % 4) {
        return -1;
    }

    size_t dstLen = 0;
    base64Decode.resize(srcLen);
    int iRet = base64_decode((const unsigned char*)src, srcLen, &base64Decode[0], &dstLen);
    if ((int)srcLen != iRet || 0 >= dstLen || dstLen >= srcLen) {
        LOG_ERROR("Failed to base64 decode, iRet: {}, dstlen: {}, srclen: {}.", iRet, dstLen, srcLen);
        return -1;
    }
    base64Decode.resize(dstLen);
    base64Decode.shrink_to_fit();

    return 0;
}

//static void add_from_bags(X509 **pX509, EVP_PKEY **pPkey, const STACK_OF(PKCS12_SAFEBAG) *bags, const char *pw);
//
//static void add_from_bag(X509 **pX509, EVP_PKEY **pPkey, PKCS12_SAFEBAG *bag, const char *pw)
//{
//    EVP_PKEY *pkey = NULL;
//    X509 *x509 = NULL;
//    switch (M_PKCS12_bag_type(bag))
//    {
//    case NID_keyBag:
//        {
//            const PKCS8_PRIV_KEY_INFO *p8 = PKCS12_SAFEBAG_get0_p8inf(bag);
//            pkey = EVP_PKCS82PKEY(p8);
//        }
//        break;
//
//    case NID_pkcs8ShroudedKeyBag:
//        {
//            PKCS8_PRIV_KEY_INFO *p8 = PKCS12_decrypt_skey(bag, pw, (int)strlen(pw));
//            if (p8)
//            {
//                pkey = EVP_PKCS82PKEY(p8);
//                PKCS8_PRIV_KEY_INFO_free(p8);
//            }
//        }
//        break;
//
//    case NID_certBag:
//        if (M_PKCS12_cert_bag_type(bag) == NID_x509Certificate)
//            x509 = PKCS12_certbag2x509(bag);
//        break;
//
//    case NID_safeContentsBag:
//        add_from_bags(pX509, pPkey, PKCS12_SAFEBAG_get0_safes(bag), pw);
//        break;
//    }
//
//    if (pkey)
//    {
//        if (!*pPkey)
//            *pPkey = pkey;
//        else
//            EVP_PKEY_free(pkey);
//    }
//
//    if (x509)
//    {
//        if (!*pX509)
//            *pX509 = x509;
//        else
//            X509_free(x509);
//    }
//}
//
//static void add_from_bags(X509 **pX509, EVP_PKEY **pPkey, const STACK_OF(PKCS12_SAFEBAG) *bags, const char *pw)
//{
//    int i;
//
//    for (i = 0; i < sk_PKCS12_SAFEBAG_num(bags); i++) {
//        add_from_bag(pX509, pPkey, sk_PKCS12_SAFEBAG_value(bags, i), pw);
//    }
//}
//
//static int openssl_read_pfx_from_bio(BIO *pfxbio,
//                                     const char *pw,
//                                     openssl_x509_pkey &pfx)
//{
//    memset(&pfx, 0, sizeof(pfx));
//
//    PKCS12 *p12 = NULL;
//    p12 = d2i_PKCS12_bio(pfxbio, NULL);
//    if (p12 == NULL) {
//        LOG_ERROR("Failed to d2i_PKCS12_bio, error: [{}]", opensslGetErrorString());
//        return -1;
//    }
//
//    STACK_OF(PKCS7) *asafes = NULL;
//    asafes = PKCS12_unpack_authsafes(p12);
//    if (asafes == NULL) {
//        LOG_ERROR("Failed to PKCS12_unpack_authsafes, error: [{}]", opensslGetErrorString());
//        PKCS12_free(p12);
//        return -1;
//    }
//
//    for (int i = 0; i < sk_PKCS7_num(asafes); i++)
//    {
//        PKCS7 *p7 = NULL;
//        p7 = sk_PKCS7_value(asafes, i);
//
//        STACK_OF(PKCS12_SAFEBAG) *bags = NULL;
//        int bagnid;
//        bagnid = OBJ_obj2nid(p7->type);
//        switch (bagnid)
//        {
//        case NID_pkcs7_data:
//            bags = PKCS12_unpack_p7data(p7);
//            break;
//        case NID_pkcs7_encrypted:
//            bags = PKCS12_unpack_p7encdata(p7, pw, (int)strlen(pw));
//            break;
//        default:
//            continue;
//        }
//
//        if (NULL == bags) {
//            LOG_ERROR("Failed to unpack p7, error: [{}]", opensslGetErrorString());
//            sk_PKCS7_pop_free(asafes, PKCS7_free);
//            PKCS12_free(p12);
//            return -1;
//        }
//
//        add_from_bags(&pfx.x509, &pfx.pkey, bags, pw);
//        sk_PKCS12_SAFEBAG_pop_free(bags, PKCS12_SAFEBAG_free);
//    }
//    sk_PKCS7_pop_free(asafes, PKCS7_free);
//
//    if (NULL == pfx.x509 || NULL == pfx.pkey)
//    {
//        if (NULL != pfx.x509)
//        {
//            X509_free(pfx.x509);
//            pfx.x509 = NULL;
//        }
//
//        if (NULL != pfx.pkey)
//        {
//            EVP_PKEY_free(pfx.pkey);
//            pfx.pkey = NULL;
//        }
//        PKCS12_free(p12);
//        LOG_ERROR("Failed to get pfx x509/pkey, error: [{}]", opensslGetErrorString());
//        return -1;
//    }
//
//    PKCS12_free(p12);
//    return 0;
//}
//
//int openssl_read_pfx_from_buf(const char *pfx_buf,
//                              size_t pfx_len,
//                              const char *pw,
//                              openssl_x509_pkey &pfx)
//{
//    int iRet = 0;
//
//    /**
//     * 判断是否base64 PEM
//     */
//    std::string strBase64Decode;
//    iRet = Base64Decode(pfx_buf, pfx_len, strBase64Decode);
//    if (0 == iRet) {
//        // pfx是base64编码的也就是PEM格式
//        LOG_DEBUG("Pfx is base64 PEM format.");
//        pfx_buf = &strBase64Decode[0];
//        pfx_len = strBase64Decode.size();
//    }
//
//    /**
//     * 解码PFX
//     */
//    BIO *pfxbio = NULL;
//    pfxbio = BIO_new_mem_buf(pfx_buf, pfx_len);
//    if (NULL == pfxbio)
//    {
//        LOG_ERROR("Failed to bio new mem size: {}.", pfx_len);
//        return -1;
//    }
//
//    iRet = openssl_read_pfx_from_bio(pfxbio, pw, pfx);
//    if (0 != iRet)
//    {
//        std::string strPfx(pfx_buf, pfx_buf + pfx_len);
//        LOG_ERROR("Failed to read pfx: [{}], password: [{}]", strPfx, pw);
//        BIO_free(pfxbio);
//        pfxbio = NULL;
//        return iRet;
//    }
//
//    BIO_free(pfxbio);
//    pfxbio = NULL;
//    return 0;
//}
//
//int openssl_read_pfx_from_path(const char *pfile,
//                               const char *pw,
//                               openssl_x509_pkey &pfx)
//{
//    std::ifstream ifs(pfile);
//    if (!ifs) {
//        LOG_ERROR("Failed to read file: {}", pfile);
//        return -1;
//    }
//
//    std::stringstream ss;
//    ss << ifs.rdbuf();
//    std::string fileBuf(ss.str());
//    if (fileBuf.empty()) {
//        LOG_ERROR("Failed to read file [{}] or file is empty.", pfile);
//        return -1;
//    }
//
//    int iRet = openssl_read_pfx_from_buf(&fileBuf[0], fileBuf.size(), pw, pfx);
//    if (0 != iRet)
//    {
//        LOG_ERROR("Failed to read pfx from file: {}.", pfile);
//        return iRet;
//    }
//
//    return 0;
//}
//
//int openssl_read_pfx(const std::string &pfxInfo,
//                     const char *pw,
//                     openssl_x509_pkey &pfx)
//{
//    if (pfxInfo.empty()) {
//        LOG_ERROR("Empty pfx info.");
//        return -1;
//    }
//
//    if (256 >= pfxInfo.size()) {
//        if (!fileExists(pfxInfo)) {
//            LOG_ERROR("Pfx file path: [{}] is not exitst.", pfxInfo);
//            return -1;
//        }
//
//        return openssl_read_pfx_from_path(pfxInfo.c_str(), pw, pfx);
//    } else {
//        return openssl_read_pfx_from_buf(pfxInfo.c_str(), pfxInfo.size(), pw, pfx);
//    }
//
//    return 0;
//}
//
//void openssl_free_pfx(const openssl_x509_pkey &pfx)
//{
//    if (NULL != pfx.x509)
//    {
//        X509_free(pfx.x509);
//    }
//
//    if (NULL != pfx.pkey)
//    {
//        EVP_PKEY_free(pfx.pkey);
//    }
//
//    return;
//}
//
///**
// * @brief 转换ASN1_TIME到time_t
// * @param [IN] time
// * @return time_t
// * @note 没有转换时区, 本地时间需要加上时区
// */
//static time_t ASN1_GetUTCTimeWithNoZone(const ASN1_TIME *time)
//{
//    struct tm t = {0};
//    const char *str = (const char*) time->data;
//    if (time->type == V_ASN1_UTCTIME) /* YYMMDDHHMMSSZ */
//    {
//        const std::string strTime = std::string("20") + str;
//        strptime(strTime.c_str(), "%Y%m%d%H%M%SZ", &t);
//    }
//    else if (time->type == V_ASN1_GENERALIZEDTIME) /* YYYYMMDDHHMMSSZ */
//    {
//        strptime(str, "%Y%m%d%H%M%SZ", &t);
//    }
//    return mktime(&t);
//}
//
//int openssl_get_x509_valid_time(const X509 *x509,
//                                time_t &startTime,
//                                time_t &endTime)
//{
//    ASN1_TIME *start = NULL;
//    ASN1_TIME *end = NULL;
//    start = X509_get_notBefore(x509);
//    end = X509_get_notAfter(x509);
//
//    // 转换到本地时间, 加8个时区
//    startTime = ASN1_GetUTCTimeWithNoZone(start) + 8 * 3600;
//    endTime = ASN1_GetUTCTimeWithNoZone(end) + 8 * 3600;
//
//    return 0;
//}
//
//int CheckPfxCertValid(const std::string &pfxData,
//                      const std::string &password,
//                      time_t* const pStartTime,
//                      time_t* const pEndTime,
//                      X509 *rootCert,
//                      X509_CRL *Crl,
//                      bool *certValid)
//{
//    int iRet = 0;
//
//    // 读取PFX
//    openssl_x509_pkey pfx = {0};
//    iRet = openssl_read_pfx_from_buf(&pfxData[0],
//                                     pfxData.size(),
//                                     password.c_str(),
//                                     pfx);
//    if (0 != iRet)
//    {
//        LOG_ERROR("Failed to read pfx.");
//        return iRet;
//    }
//
//    // 读取证书有效期
//    time_t startTime;
//    time_t endTime;
//    (void)openssl_get_x509_valid_time(pfx.x509,
//                                      startTime,
//                                      endTime);
//
//    if (NULL != pStartTime) {
//        *pStartTime = startTime;
//    }
//
//    if (NULL != pEndTime) {
//        *pEndTime = endTime;
//    }
//
//    time_t nowTime = time(NULL);
//    if (nowTime < startTime || nowTime > endTime) {
//        LOG_ERROR("Pfx cert is invalid time, now time: [{}], cert start time: [{}], end time: [{}]",
//                  nowTime, startTime, endTime);
//        openssl_free_pfx(pfx);
//        return -1;
//    }
//    LOG_DEBUG("Now time: {}, Start time: {}, end time: {}.", nowTime, startTime, endTime);
//
//    // 检查证书是否吊销
//    if (NULL != rootCert && NULL != Crl && NULL != certValid)
//    {
//        iRet = CRL_Verify(rootCert, Crl, pfx.x509, *certValid);
//        if (0 != iRet) {
//            LOG_ERROR("Failed to crl verify.");
//            openssl_free_pfx(pfx);
//            return iRet;
//        }
//    }
//
//    openssl_free_pfx(pfx);
//    return 0;
//}
//
//static void SM3_KDF(const std::string &input, char output[32])
//{
//    memset(output, 0, (int)sizeof(output));
//
//    char ct[4] = {0, 0, 0, 1};
//    SM3_STATE md;
//    SM3_init(&md);
//    SM3_process(&md, (unsigned char*)input.c_str(), input.size());
//    SM3_process(&md, (unsigned char*)ct, sizeof(ct));
//    SM3_done(&md, (unsigned char*)&output[0]);
//    return;
//}
//
//int SM4_DAO_decrypt(const std::string &inputEncrypt,
//                    const std::string &password,
//                    std::string &decryptOutput)
//{
//    int iRet = 0;
//
//    // 两次base64解码
//    std::string base64Decode;
//    iRet = Base64Decode(inputEncrypt.c_str(), inputEncrypt.size(), base64Decode);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to base64 decode: [{}]", inputEncrypt);
//        return iRet;
//    }
//
//    std::string base64Decode2;
//    iRet = Base64Decode(&base64Decode[0], base64Decode.size(), base64Decode2);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to base64 decode: [{}]", base64Decode);
//        return iRet;
//    }
//
//    // 解密过程
//    char output[32] = {0};
//    SM3_KDF(password, output);
//
//    // 前16字节iv
//    char iv[16] = {0};
//    memcpy(iv, output, 16);
//
//    // 后16字节key
//    char key[16] = {0};
//    memcpy(key, output + 16, 16);
//
//    iRet = SM4DecryptCBC(base64Decode2,
//                         key,
//                         iv,
//                         decryptOutput);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to sm4 decrypt cbc: [{}].", base64Decode2);
//        return iRet;
//    }
//
//    return 0;
//}
//
//int SM4_DAO_encrypt(const std::string &inputSouce,
//                    const std::string &password,
//                    std::string &encryptOutput)
//{
//    /**
//     * SM4加密
//     */
//    char output[32] = {0};
//    SM3_KDF(password, output);
//
//    // 前16字节iv
//    char iv[16] = {0};
//    memcpy(iv, output, 16);
//
//    // 后16字节key
//    char key[16] = {0};
//    memcpy(key, output + 16, 16);
//
//    (void)SM4EncryptCBC(inputSouce, key, iv, encryptOutput);
//
//    /**
//     * 两次base64加密
//     */
//    std::string base64Encrypt1 = Base64Encode(encryptOutput.c_str(), encryptOutput.size());
//    encryptOutput = Base64Encode(base64Encrypt1.c_str(), base64Encrypt1.size());
//
//    return 0;
//}
//
//////////////////////////////////////////////////////////////////
///////////////////////////////X509证书接口////////////////////////
//////////////////////////////////////////////////////////////////
//static X509 *LoadX509FromBio(BIO *bio, const char *format)
//{
//    X509 *x509 = NULL;
//    if ("PEM" == std::string(format)) {
//        x509 = PEM_read_bio_X509(bio, NULL, 0, NULL);
//    } else if ("ASN1" == std::string(format)) {
//        x509 = d2i_X509_bio(bio, NULL);
//    } else {
//        LOG_ERROR("Formate must [PEM/ASN1]");
//        return NULL;
//    }
//
//    if (NULL == x509)
//    {
//        LOG_ERROR("Failed to load x509 format: {}.", format);
//        return NULL;
//    }
//
//    return x509;
//}
//
//X509 *LoadX509FromPath(const char *x509Path, const char *format)
//{
//
//    BIO *bio = NULL;
//    bio = BIO_new_file(x509Path, "rb");
//    if (NULL == bio)
//    {
//        LOG_ERROR("Failed to bio new file path: {}.", x509Path);
//        return NULL;
//    }
//
//    X509 *x509 = LoadX509FromBio(bio, format);
//    if (NULL == x509) {
//        LOG_ERROR("Failed to load x509 from path: {}, format: {}.",
//                  x509Path, format);
//        BIO_free(bio);
//        bio = NULL;
//        return NULL;
//    }
//
//    BIO_free(bio);
//    bio = NULL;
//    return x509;
//}
//
//X509 *LoadX509FromBuf(const char *x509Buf, size_t x509Len, const char *format)
//{
//
//    BIO *bio = NULL;
//    bio = BIO_new_mem_buf(x509Buf, x509Len);
//    if (NULL == bio)
//    {
//        LOG_ERROR("Failed to bio new mem size: {}.", x509Len);
//        return NULL;
//    }
//
//    X509 *x509 = LoadX509FromBio(bio, format);
//    if (NULL == x509) {
//        LOG_ERROR("Failed to load x509 from buf size: {}, format: {}.",
//                  x509Len, format);
//        BIO_free(bio);
//        bio = NULL;
//        return NULL;
//    }
//
//    BIO_free(bio);
//    bio = NULL;
//    return x509;
//}
//
//void FreeX509(X509 *x509)
//{
//    if (NULL != x509) {
//        X509_free(x509);
//    }
//}
//
//////////////////////////////////////////////////////////////////
///////////////////////////////CRL接口////////////////////////////
//////////////////////////////////////////////////////////////////
//static X509_CRL *LoadCRLFromBio(BIO *bio, const char *format)
//{
//    X509_CRL *crl = NULL;
//    if ("PEM" == std::string(format)) {
//        crl = PEM_read_bio_X509_CRL(bio, NULL, NULL, NULL);
//    } else if ("ASN1" == std::string(format)) {
//        crl = d2i_X509_CRL_bio(bio, NULL);
//    } else {
//        LOG_ERROR("Formate must [PEM/ASN1]");
//        return NULL;
//    }
//
//    if (NULL == crl)
//    {
//        LOG_ERROR("Failed to load crl format: {}.", format);
//        return NULL;
//    }
//
//    return crl;
//}
//
//X509_CRL *LoadCRLFromPath(const char *crlPath, const char *format)
//{
//
//    BIO *bio = NULL;
//    bio = BIO_new_file(crlPath, "rb");
//    if (NULL == bio)
//    {
//        LOG_ERROR("Failed to bio new file path: {}.", crlPath);
//        return NULL;
//    }
//
//    X509_CRL *crl = LoadCRLFromBio(bio, format);
//    if (NULL == crl) {
//        LOG_ERROR("Failed to load crl for path: {}.", crlPath);
//        BIO_free(bio);
//        bio = NULL;
//        return NULL;
//    }
//
//    BIO_free(bio);
//    bio = NULL;
//    return crl;
//}
//
//X509_CRL *LoadCRLFromBuf(const char *crlBuf, size_t crlLen, const char *format)
//{
//
//    BIO *bio = NULL;
//    bio = BIO_new_mem_buf(crlBuf, crlLen);
//    if (NULL == bio)
//    {
//        LOG_ERROR("Failed to bio new mem size: {}.", crlLen);
//        return NULL;
//    }
//
//    X509_CRL *crl = LoadCRLFromBio(bio, format);
//    if (NULL == crl) {
//        LOG_ERROR("Failed to load crl buf size: {}, format: {}.",
//                  crlLen, format);
//        BIO_free(bio);
//        bio = NULL;
//        return NULL;
//    }
//
//    BIO_free(bio);
//    bio = NULL;
//    return crl;
//}
//
//void FreeX509CRL(X509_CRL *crl)
//{
//    if (NULL != crl) {
//        X509_CRL_free(crl);
//    }
//    return;
//}
//
//int CRL_Verify(X509 *rootCert,
//               X509_CRL *Crl,
//               X509 *userCert,
//               bool &certValid)
//{
//    certValid = false;
//    int iRet = 0;
//
//    // 加载根证书, 证书库，存在证书链
//    X509_STORE *rootCertStore = NULL;
//    rootCertStore = X509_STORE_new();
//    X509_STORE_add_cert(rootCertStore, rootCert);
//
//    // 设置CRL查询标记,添加CRL
//    X509_STORE_set_flags(rootCertStore, X509_V_FLAG_CRL_CHECK);
//    X509_STORE_add_crl(rootCertStore, Crl);
//
//    // 加载证书存储上下文
//    X509_STORE_CTX *ctx = NULL;
//    ctx = X509_STORE_CTX_new();
//
//    STACK_OF(X509) *caCertStack = NULL;
//    iRet = X509_STORE_CTX_init(ctx, rootCertStore, userCert, caCertStack);
//    if(iRet != 1)
//    {
//        LOG_ERROR("Failed to X509_STORE_CTX_init.");
//        X509_STORE_CTX_cleanup(ctx);
//        X509_STORE_CTX_free(ctx);
//        X509_STORE_free(rootCertStore);
//        return -1;
//    }
//
//    iRet = X509_verify_cert(ctx);
//    if(iRet != 1)
//    {
//        LOG_DEBUG("Failed to verify cert.");
//        certValid = false;
//        X509_STORE_CTX_cleanup(ctx);
//        X509_STORE_CTX_free(ctx);
//        X509_STORE_free(rootCertStore);
//        return 0;
//    }
//
//    certValid = true;
//    X509_STORE_CTX_cleanup(ctx);
//    X509_STORE_CTX_free(ctx);
//    X509_STORE_free(rootCertStore);
//
//    return 0;
//}
//
//int CRL_VerifyFromPath(const char *rootCertPath,
//                       const char *crlPath,
//                       const char *userCertPath,
//                       bool &certValid)
//{
//    X509 *rootCert = LoadX509FromPath(rootCertPath);
//    if (NULL == rootCert) {
//        LOG_ERROR("Failed to load x509 from path: {}.", rootCertPath);
//        return -1;
//    }
//
//    X509_CRL *Crl = LoadCRLFromPath(crlPath);
//    if (NULL == Crl) {
//        LOG_ERROR("Failed to load CRL from path: {}.", crlPath);
//        FreeX509(rootCert);
//        return -1;
//    }
//
//    X509 *userCert = LoadX509FromPath(userCertPath);
//    if (NULL == userCert) {
//        LOG_ERROR("Failed to load x509 from path: {}.", userCertPath);
//        FreeX509(rootCert);
//        FreeX509CRL(Crl);
//        return -1;
//    }
//
//    int iRet = CRL_Verify(rootCert, Crl, userCert, certValid);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to crl verify: root cert path: {}, crl path: {}, user cert paht: {}.",
//                  rootCertPath, crlPath, userCertPath);
//        FreeX509(rootCert);
//        FreeX509CRL(Crl);
//        FreeX509(userCert);
//        return iRet;
//    }
//
//    FreeX509(rootCert);
//    FreeX509CRL(Crl);
//    FreeX509(userCert);
//    return 0;
//}
//
//static void opensslPrintX509Name(std::string &x509Name,
//                                 X509_NAME *nm,
//                                 unsigned long lflags = XN_FLAG_COMPAT)
//{
//    BIO *bio_out = BIO_new(BIO_s_mem());
//    if(NULL == bio_out) {
//        return;
//    }
//
//    char *buf = NULL;
//    char mline = 0;
//    int indent = 0;
//
//    if ((lflags & XN_FLAG_SEP_MASK) == XN_FLAG_SEP_MULTILINE) {
//        mline = 1;
//        indent = 4;
//    }
//    if (lflags == XN_FLAG_COMPAT) {
//        buf = X509_NAME_oneline(nm, 0, 0);
//        BIO_puts(bio_out, buf);
//        OPENSSL_free(buf);
//    } else {
//        if (mline) {
//            BIO_puts(bio_out, "\n");
//        }
//        X509_NAME_print_ex(bio_out, nm, indent, lflags);
//    }
//
//    BUF_MEM *biomem = NULL;
//    BIO_get_mem_ptr(bio_out, &biomem);
//    x509Name.assign(biomem->data, biomem->data + biomem->length);
//    BIO_free(bio_out);
//    bio_out = NULL;
//}
//
//int GetX509NameSubject(const X509 *x509, std::string &x509NameSubject)
//{
//    // 格式: /C=cn/ST=cq/O=zl/OU=zl/CN=user.zl/emailAddress=zl03jsj@163.com
//    std::string x509Name;
//    opensslPrintX509Name(x509Name, X509_get_subject_name(x509), XN_FLAG_COMPAT);
//
//    std::map<std::string, std::string> subjectMap;
//    std::istringstream ss(x509Name);
//    for (std::string item; std::getline(ss, item, '/'); ) {
//        if (item.empty()) {
//            continue;
//        }
//        size_t pos = item.find("=");
//        if (std::string::npos == pos) {
//            continue;
//        }
//        std::string key = item.substr(0, pos);
//        std::string value = item.substr(pos + 1);
//        subjectMap[key] = value;
//    }
//
//    // 格式: E=zl03jsj@163.com, CN=user.zl, OU=zl, O=zl, ST=cq, C=cn
//    x509NameSubject = std::string("E=") + subjectMap["emailAddress"] + ", " +
//                      std::string("CN=") + subjectMap["CN"] + ", " +
//                      std::string("OU=") + subjectMap["OU"] + ", " +
//                      std::string("O=") + subjectMap["O"] + ", " +
//                      std::string("ST=") + subjectMap["ST"] + ", " +
//                      std::string("C=") + subjectMap["C"];
//
//    return 0;
//}
//
//int GetPfxCertNameSubject(const std::string &pfx,
//                          const std::string &pfxPasswd,
//                          std::string &x509NameSubject)
//{
//    int iRet = 0;
//
//    openssl_x509_pkey pfxPkey = {0};
//    iRet = openssl_read_pfx_from_buf(pfx.c_str(), pfx.size(), pfxPasswd.c_str(), pfxPkey);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to read pfx.");
//        openssl_free_pfx(pfxPkey);
//        return iRet;
//    }
//
//    iRet = GetX509NameSubject(pfxPkey.x509, x509NameSubject);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to get x509 subject.");
//        openssl_free_pfx(pfxPkey);
//        return iRet;
//    }
//
//    openssl_free_pfx(pfxPkey);
//
//    return 0;
//}
//
//std::string X509_GetSignatureOIDText(const X509 *x509)
//{
//    int nid = X509_get_signature_nid(x509);
//    ASN1_OBJECT *object = OBJ_nid2obj(nid);
//    if (NULL == object) {
//        LOG_ERROR("Failed to get object from nid.");
//        return std::string();
//    }
//
//    const int oidLen = 128;
//    char oid[oidLen] = {0};
//    OBJ_obj2txt(oid, oidLen, object, 1);
//
//    std::string x509OID;
//    x509OID.resize(oidLen);
//    memcpy(&x509OID[0], oid, oidLen);
//
//    return x509OID;
//}
//
//#if 0
//int PFX_GetKeyType(const std::string &pfxData, std::string &keyType)
//{
//    // sm2
//    const unsigned char sm2OID[] = {0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x06, 0x01, 0x04, 0x02, 0x01};
//
//    // openssl: [  115] OBJ_pkcs7_data
//    // 1.2.840.113549.1.7.1 data (PKCS #7)
//    const unsigned char rsaOID[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01};
//
//    int rootLength = pfxData[1] & 15;
//    int seqLength = pfxData[6 + rootLength];
//    int seqLengthLength = 1;
//    if (seqLength < 0) {
//        seqLengthLength = seqLengthLength + (seqLength & 15);
//    }
//
//    int p12Flag = pfxData[seqLengthLength + 6 + rootLength];
//    if (p12Flag != 6) {
//        LOG_ERROR("Not p12 type.");
//        return -1;
//    }
//
//    int oidLength = pfxData[seqLengthLength + 7 + rootLength];
//    int start = seqLengthLength + 8 + rootLength;
//
//    std::string oidData;
//    oidData.resize(oidLength);
//    memcpy(&oidData[0], &pfxData[0] + start, oidLength);
//
//    if (oidLength == sizeof(sm2OID) &&
//        0 == memcmp((unsigned char*)sm2OID, (unsigned char*)&oidData[0], oidLength)) {
//        keyType = "sm2";
//        return 0;
//    } else if (oidLength == sizeof(rsaOID) &&
//               0 == memcmp((unsigned char*)rsaOID, (unsigned char*)&oidData[0], oidLength)) {
//        keyType = "rsa";
//        return 0;
//    } else {
//        LOG_ERROR("Unkown oid.");
//        return -1;
//    }
//
//    return 0;
//}
//#endif
//
//std::string GetPKeyType(EVP_PKEY *pkey)
//{
//    int id = EVP_PKEY_base_id(pkey);
//    if (6 == id) {
//        // Public Key Algorithm: rsaEncryption
//        return "rsa";
//    } else if (408 == id) {
//        // Public Key Algorithm: id-ecPublicKey
//        return "sm2";
//    }
//
//    return std::string();
//}
//
//int GetPfxPKeyType(const std::string &pfx,
//                   const std::string &pfxPasswd,
//                   std::string &pkeyType)
//{
//    int iRet = 0;
//
//    openssl_x509_pkey pfxPkey = {0};
//    iRet = openssl_read_pfx_from_buf(pfx.c_str(), pfx.size(), pfxPasswd.c_str(), pfxPkey);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to read pfx.");
//        openssl_free_pfx(pfxPkey);
//        return iRet;
//    }
//
//    pkeyType = GetPKeyType(pfxPkey.pkey);
//    if (pkeyType.empty()) {
//        LOG_ERROR("Failed to get pkey type.");
//        openssl_free_pfx(pfxPkey);
//        return -1;
//    }
//
//    openssl_free_pfx(pfxPkey);
//
//    return 0;
//}
//
//int GetX509NameIssuer(const X509 *x509, std::string &x509NameIssuer)
//{
//    opensslPrintX509Name(x509NameIssuer, X509_get_issuer_name(x509));
//    if (x509NameIssuer.empty()) {
//        LOG_ERROR("Empty x509 name issuer.");
//        return -1;
//    }
//
//    return 0;
//}
//
//int GetPfxCertNameIssuer(const std::string &pfx,
//                         const std::string &pfxPasswd,
//                         std::string &x509NameIssuer)
//{
//    int iRet = 0;
//
//    openssl_x509_pkey pfxPkey = {0};
//    iRet = openssl_read_pfx_from_buf(pfx.c_str(), pfx.size(), pfxPasswd.c_str(), pfxPkey);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to read pfx.");
//        openssl_free_pfx(pfxPkey);
//        return iRet;
//    }
//
//    iRet = GetX509NameIssuer(pfxPkey.x509, x509NameIssuer);
//    if (0 != iRet) {
//        LOG_ERROR("Failed to get x509 issuer.");
//        openssl_free_pfx(pfxPkey);
//        return iRet;
//    }
//
//    openssl_free_pfx(pfxPkey);
//
//    return 0;
//}
//
//static CMS_ContentInfo *CMS_get_signerinfo(BIO *bioP7Info, CMS_SignerInfo *&sigInfo)
//{
//    // 读取pk7数据到CMS
//    CMS_ContentInfo *cms = d2i_CMS_bio(bioP7Info, NULL);
//    if (NULL == cms)
//    {
//        LOG_ERROR("Failed to new cms.");
//        return NULL;
//    }
//
//    // 获得签名信息
//    STACK_OF(CMS_SignerInfo) *sis = NULL;
//    sis = CMS_get0_SignerInfos(cms);
//    if (NULL == sis)
//    {
//        CMS_ContentInfo_free(cms);
//        LOG_ERROR("Failed to get signer info.\n");
//        return NULL;
//    }
//
//    sigInfo = NULL;
//    sigInfo = sk_CMS_SignerInfo_value(sis, 0);
//    if (NULL == sigInfo)
//    {
//        CMS_ContentInfo_free(cms);
//        LOG_ERROR("Failed to cms signer info value.\n");
//        return NULL;
//    }
//
//    return cms;
//}
//
//int PKCS7_get_signatureinfo_issuer(const std::string &pkcs7, std::string &issuer)
//{
//    BIO *bioPkcs7 = BIO_new_mem_buf(&pkcs7[0], pkcs7.size());
//    if (NULL == bioPkcs7)
//    {
//        LOG_ERROR("Failed to new bio mem buf.");
//        return -1;
//    }
//
//    CMS_SignerInfo *sigInfo = NULL;
//    CMS_ContentInfo *cms = CMS_get_signerinfo(bioPkcs7, sigInfo);
//    if (NULL == cms || NULL == sigInfo) {
//        LOG_ERROR("Failed to get signer info.");
//        BIO_free(bioPkcs7);
//        bioPkcs7 = NULL;
//        return -1;
//    }
//
//    X509_NAME *issuerName = NULL;
//    CMS_SignerInfo_get0_signer_id(sigInfo, NULL, &issuerName, NULL);
//    if (NULL == issuerName) {
//        LOG_ERROR("Failed to get0 signer id.");
//        BIO_free(bioPkcs7);
//        bioPkcs7 = NULL;
//        CMS_ContentInfo_free(cms);
//        cms = NULL;
//        return -1;
//    }
//
//    opensslPrintX509Name(issuer, issuerName);
//
//    BIO_free(bioPkcs7);
//    bioPkcs7 = NULL;
//    CMS_ContentInfo_free(cms);
//    cms = NULL;
//    return 0;
//}
//
//int PKCS7_get_info(PKCS7 *pkcs7, std::string &p7Info)
//{
//    BIO *bioP7Sig = BIO_new(BIO_s_mem());
//    if (bioP7Sig == NULL || !i2d_PKCS7_bio(bioP7Sig, pkcs7))
//    {
//        LOG_ERROR("Failed to pkcs7 to bio.");
//        BIO_free(bioP7Sig);
//        bioP7Sig = NULL;
//        return -1;
//    }
//
//    char *p7Buf = NULL;
//    int bufLen = BIO_get_mem_data(bioP7Sig, &p7Buf);
//    if (NULL == p7Buf || 0 >= bufLen)
//    {
//        LOG_ERROR("Failed to bio get mem data.");
//        BIO_free(bioP7Sig);
//        bioP7Sig = NULL;
//        return -1;
//    }
//    p7Info.assign(p7Buf, bufLen);
//
//    BIO_free(bioP7Sig);
//    bioP7Sig = NULL;
//    return 0;
//}

} /* namespace util */
