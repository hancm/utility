#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ice.h"
#include "encode.h"
#include "compress.h"
#include "encrypt.h"

///**
// * @brief 获取字符串二进制的比特位数值
// * @param [IN] s            字符串
// * @param [IN] limit        字符串长度
// * @param [IN] n            需要获取的比特位位置(最大8*limit)
// * @return int
// * @note
// */
//inline int get_string_nbit(const char *s, int limit, int n)
//{
//    int byte = n >> 3;
//    int bit = n & 7;
//
//    if (byte < 0 || byte > limit) {
//        return -1;
//    }
//
//    return (s[byte] & (1 << bit)) >> bit;
//}
//
///**
// * @brief 设置字符串相应二进制位置的值
// * @param [IN] s            字符串
// * @param [IN] limit        字符串长度
// * @param [IN] n            需要修改的字符串比特位数(最大8*limit)
// * @param [IN] v            相应比特位值(0/1)
// * @return int
// * @note
// */
//inline int set_string_bit(char *s, int limit, int n, int v)
//{
//    int byte = n >> 3;
//    int bit = n & 7;
//
//    if (byte < 0 || byte > limit) {
//        return -1;
//    }
//
//    if (v) {
//      s[byte] |= (1 << bit);
//    } else {
//      s[byte] &= ~(1 << bit);
//    }
//
//    return 0;
//}
//
///**
// * @brief 0/1组成的字符串转换为char类型的字符串
// * @param [IN] binStr           0/1组成的字符串
// * @param [IN] strOutputBuf     char类型字符串
// * @param [IN] bits             char类型字符串中bit为数目(0/1组成的字符串可能不是8整数倍)
// * @return int
// * @note
// */
//int binstr_to_string(const std::string &binStr,
//                     std::string &strOutputBuf,
//                     int &bits)
//{
//    strOutputBuf.resize(binStr.size());
//
//    int bitsCount = 0;
//    for (size_t i = 0; i < binStr.size(); i++) {
//        if (binStr[i] == '0') {
//            set_string_bit(&strOutputBuf[0], strOutputBuf.size(), i, 0);
//            ++bitsCount;
//        } else if (binStr[i] == '1') {
//            set_string_bit(&strOutputBuf[0], strOutputBuf.size(), i, 1);
//            ++bitsCount;
//        } else {
//            continue;
//        }
//    }
//
//    bits = bitsCount;
//
//    int bytes = (bitsCount % 8 > 0) ? bitsCount / 8 + 1 : bitsCount / 8;
//    strOutputBuf.resize(bytes);
//    strOutputBuf.shrink_to_fit();
//
//    return 0;
//}
//
///**
// * @brief 将char类型字符串转换为0/1组成的字符串
// * @param [IN] strInputBuf      char字符串
// * @param [IN] bitsNum          需要转换的bit数目(最大为8倍char字符串长度)
// * @param [IN] binStr           转换后的0/1组成的字符串
// * @return int
// * @note
// */
//int string_to_binstr(const std::string &strInputBuf, int bitsNum, std::string &binStr)
//{
//    binStr.resize(bitsNum);
//    for (int i = 0; i < bitsNum; i++) {
//        binStr[i] = get_string_nbit(strInputBuf.c_str(), strInputBuf.size(), i) ? '1' : '0';
//    }
//
//    return 0;
//}

static void char_to_binstr(unsigned char c, std::string &binString)
{
    for (int i = 0; i < 8; i++)
    {
        char bit = ((c & (128 >> i)) != 0) ? '1' : '0';
        binString.push_back(bit);
    }
}

void string_to_binstr(const std::string &charString, std::string &binString)
{
    for (size_t i = 0; i < charString.size(); ++i)
    {
        char_to_binstr(charString[i], binString);
    }
}

static
void output_bit(int bit,
                int &output_bit_count,
                int &output_value,
                std::string &charString)
{
    output_value = (output_value << 1) | bit;
    if (++output_bit_count == 8) {
        charString.push_back((char)output_value);
        output_value = 0;
        output_bit_count = 0;
    }
}

int binstr_to_string(const std::string &binString, std::string &charString)
{
    charString.clear();
    if (0 != binString.size() % 8)
    {
        return -1;
    }

    int output_bit_count = 0;
    int output_value = 0;
    for (size_t i = 0; i < binString.size(); ++i)
    {
        int bit = 0;
        if ('0' == binString[i]) {
            bit = 0;
        } else if ('1' == binString[i]) {
            bit = 1;
        } else {
            return -1;
        }
        output_bit(bit, output_bit_count, output_value, charString);
    }

    return 0;
}

int main(void)
{
    int iRet = 0;

    /**
     * ice加解密
     */
    /*
    char* pw = "123hancm456";
    unsigned char ptxt[9] = "ha韩长";
    unsigned char ctxt[9] = {0};

    ICE_KEY *ice = ice_key_create();
    ice_key_set(ice, (unsigned char*)pw);

    ice_key_encrypt(ice, ptxt, ctxt);
    std::cout << "encrypt: " << ctxt << std::endl;

    ice_key_decrypt (ice, ctxt, ptxt);
    std::cout << "decrypt: " << ptxt << std::endl;

    ice_key_destroy (ice);
    */

    /**
     * encode编解码
     */
    // 编码
//  std::string encodeBuffer;
//  iRet = message_string_encode("2232ddd123456789hancm123243647560-0-08090028804804hancm韩长鸣", encodeBuffer);
//  std::cout << "Ret: " << iRet << " encode buffer size: " << encodeBuffer.size() << std::endl;
//
//  std::ofstream of("encode.txt");
//  of.write(encodeBuffer.c_str(), encodeBuffer.size());
//
//  // 解码
//  std::string encode_output;
//  iRet = message_extract (encodeBuffer, encode_output);
//  std::cout << "Ret: " << iRet << " decode: " << encode_output << std::endl;

    /**
     * 压缩
     */
//  std::string compress_output_string;
//  compress_string("hancm", compress_output_string);
//  std::cout << "output: " << compress_output_string << std::endl;
//
//  int bits = 0;
//  std::string strOutputBuf;
//  binstr_to_string(compress_output_string, strOutputBuf, bits);
//  std::cout << "string: " << strOutputBuf << " Bits: " << bits << std::endl;
//
//  std::string binStr;
//  string_to_binstr(strOutputBuf, strOutputBuf.size() * 8, binStr);
//  compress_output_string = binStr;
//  std::cout << "compress: " << compress_output_string << std::endl;
//
//  COMPRESS_STATUS_S compress_status;
//  uncompress_init(compress_status);
//  for (int i = 0; i < compress_output_string.size(); ++i) {
//      int bit = 0;
//      if ('0' == compress_output_string[i]) {
//          bit = 0;
//      } else if ('1' == compress_output_string[i]) {
//          bit = 1;
//      }
//      uncompress_bit(compress_status, bit);
//  }
//
//  std::string uncompress_out;
//  uncompress_flush(compress_status, uncompress_out);
//  std::cout << "uncompress_out: " << uncompress_out << std::endl;

    /**
     * 加解密
     */
    std::string binString;
    string_to_binstr("hancm韩长鸣", binString);
    std::cout << "binString: " << binString << std::endl;

    // 加密
    ENCRYPT_STATUS_S encrypt_status;
    encrypt_init(encrypt_status, "hancm");
    for (size_t i = 0; i < binString.size(); ++i)
    {
        int bit = 0;
        if ('0' == binString[i]) {
            bit = 0;
        } else if ('1' == binString[i]) {
            bit = 1;
        }
        encrypt_bit(encrypt_status, bit);
    }
    std::string encrypt_output_string;
    encrypt_flush(encrypt_status, encrypt_output_string);
    std::cout << "encrypt output: " << encrypt_output_string << std::endl;

//  std::string charString;
//  binstr_to_string(binString, charString);
//  std::cout << "charString: " << charString << std::endl;

    // 解密
    decrypt_init(encrypt_status, "hancm");
    for (size_t i = 0; i < encrypt_output_string.size(); ++i)
    {
        int bit = 0;
        if ('0' == encrypt_output_string[i]) {
            bit = 0;
        } else if ('1' == encrypt_output_string[i]) {
            bit = 1;
        }
        decrypt_bit(encrypt_status, bit);
    }
    std::string decrypt_output_string;
    decrypt_flush(encrypt_status, decrypt_output_string);
    std::cout << "decrypt output: " << decrypt_output_string << std::endl;

    return 0;
}