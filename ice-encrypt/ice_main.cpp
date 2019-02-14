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

int main(void)
{
    char* pw = "123hancm456";
    unsigned char ptxt[9] = "ha韩长";
    unsigned char ctxt[9] = {0};
{
    /*ICE_KEY *ice = ice_key_create();
    ice_key_set(ice, (unsigned char*)pw);

    ice_key_encrypt(ice, ptxt, ctxt);
    std::cout << "encrypt: " << ctxt << std::endl;

    ice_key_decrypt (ice, ctxt, ptxt);
    std::cout << "decrypt: " << ptxt << std::endl;

    ice_key_destroy (ice);
    */
{
    FILE *inf = stdin;
    FILE *outf = stdout;
    std::istringstream infile_stream;
//  infile_stream.str("%%EOF");

    std::ostringstream outfile_stream;

    encode_init();
    encode_bit(1, inf, outf, infile_stream, outfile_stream);

    std::string encodeBuffer;
    encode_flush(inf, outf, infile_stream, outfile_stream, encodeBuffer);

    std::cout << "infile stream: " << infile_stream.str() << "\noutfile stream: "
              << outfile_stream.str() << "\nencode buffer: " << encodeBuffer.size() << std::endl;

    std::ofstream of("encode.txt");
    of.write(encodeBuffer.c_str(), encodeBuffer.size());
}
}
    return 0;
}