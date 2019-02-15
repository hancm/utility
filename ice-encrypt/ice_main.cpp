#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ice.h"

extern void encode_init (void);

extern int encode_bit (
    int     bit,
    FILE        *inf,
    FILE        *outf,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream
);

extern int
encode_flush (
    FILE        *inf,
    FILE        *outf,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream,
    std::string &encodeBuffer
);

extern int
message_extract (
    FILE        *inf,
    FILE        *outf,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream,
    int     text_encode_mode = 1,
    char splite_char = '\r'
);

int
character_encode(unsigned char c,
                 FILE *infile,
                 FILE *outfile,
                 std::istringstream &infile_stream,
                 std::ostringstream &outfile_stream)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = ((c & (128 >> i)) != 0) ? 1 : 0;
        if (!encode_bit (bit, infile, outfile, infile_stream, outfile_stream)) {
            return (false);
        }
    }
    return (true);
}

/**
 * @brief 对消息进行加密
 * @param [IN] msg
 * @param [IN] infile
 * @param [IN] outfile
 * @return BOOL
 * @note
 */
int
message_string_encode(const char *msg,
                      FILE *infile,
                      FILE *outfile,
                      std::istringstream &infile_stream,
                      std::ostringstream &outfile_stream)
{
//  compress_init ();
    while (*msg != '\0') {
        if (!character_encode (*msg, infile, outfile, infile_stream, outfile_stream)) {
            return (false);
        }
        msg++;
    }

    return 0;//(compress_flush (infile, outfile));
}

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

    FILE *inf = stdin;
    FILE *outf = stdout;
    std::istringstream infile_stream;
    std::ostringstream outfile_stream;

    encode_init();

    message_string_encode("123456789hancm123243647560-0-08090028804804hancm韩长鸣", inf, outf, infile_stream, outfile_stream);

    std::string encodeBuffer;
    encode_flush(inf, outf, infile_stream, outfile_stream, encodeBuffer);

    std::cout << "infile stream: " << infile_stream.str() << "\noutfile stream size: "
              << outfile_stream.str().size() << "\nencode buffer size: " << encodeBuffer.size() << std::endl;

    encodeBuffer = outfile_stream.str();
    std::ofstream of("encode.txt");
    of.write(encodeBuffer.c_str(), encodeBuffer.size());

    // 解码
    infile_stream.clear();
    infile_stream.str(encodeBuffer);

    outfile_stream.clear();
    outfile_stream.str("");
    message_extract (inf, outf, infile_stream, outfile_stream);

    std::cout << "decode: " << outfile_stream.str() << std::endl;
    return 0;
}