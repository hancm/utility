#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ice.h"
#include "encode.h"

int main(void)
{
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

    int iRet = 0;

    // 编码
    std::string encodeBuffer;
    iRet = message_string_encode("2232ddd123456789hancm123243647560-0-08090028804804hancm韩长鸣232323232323", encodeBuffer);
    std::cout << "Ret: " << iRet << " encode buffer size: " << encodeBuffer.size() << std::endl;

    std::ofstream of("encode.txt");
    of.write(encodeBuffer.c_str(), encodeBuffer.size());

    // 解码
    std::string encode_output;
    iRet = message_extract (encodeBuffer, encode_output);
    std::cout << "Ret: " << iRet << " decode: " << encode_output << std::endl;
    return 0;
}