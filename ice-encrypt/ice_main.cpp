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

    ENCODE_STATUS_S encode_status;
    encode_init(encode_status);

    message_string_encode(encode_status, "123456789hancm123243647560-0-08090028804804hancm韩长鸣");

    std::string encodeBuffer;
    encode_flush(encode_status, encodeBuffer);

    std::cout << "encode buffer size: " << encodeBuffer.size() << std::endl;

    std::ofstream of("encode.txt");
    of.write(encodeBuffer.c_str(), encodeBuffer.size());

    // 解码
    std::istringstream infile_stream;
    infile_stream.str(encodeBuffer);

    std::ostringstream outfile_stream;
    outfile_stream.clear();
    outfile_stream.str("");
    message_extract (infile_stream, outfile_stream);

    std::cout << "decode: " << outfile_stream.str() << std::endl;
    return 0;
}