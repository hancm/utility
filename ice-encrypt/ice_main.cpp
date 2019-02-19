#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ice.h"
#include "encode.h"
#include "compress.h"

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
//  // 编码
//  std::string encodeBuffer;
//  iRet = message_string_encode("2232ddd123456789hancm123243647560-0-08090028804804hancm韩长鸣232323232323", encodeBuffer);
//  std::cout << "Ret: " << iRet << " encode buffer size: " << encodeBuffer.size() << std::endl;
//
//  std::ofstream of("encode.txt");
//  of.write(encodeBuffer.c_str(), encodeBuffer.size());
//
//  // 解码
//  std::string encode_output;
//  iRet = message_extract (encodeBuffer, encode_output);
//  std::cout << "Ret: " << iRet << " decode: " << encode_output << std::endl;

    compress_init();
    compress_character('h');

    std::string compress_output_string;
    compress_flush(compress_output_string);
    std::cout << "output: " << compress_output_string << std::endl;

    std::string uncompress_out;
    uncompress_init();
    for (int i = 0; i < compress_output_string.size(); ++i) {
        int bit = 0;
        if ('0' == compress_output_string[i]) {
            bit = 0;
        } else if ('1' == compress_output_string[i]) {
            bit = 1;
        }
        uncompress_bit(bit, uncompress_out);
    }
    std::cout << "uncompress_out: " << uncompress_out << std::endl;

    return 0;
}