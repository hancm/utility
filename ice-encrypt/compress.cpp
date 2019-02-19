#include <string.h>
#include <string>
#include <iostream>

static const char *huffcodes[256] = {
#include "huffcode.h"
};

static int              compress_bit_count;
static int              compress_value;
static unsigned long    compress_bits_in;
static unsigned long    compress_bits_out;
static std::string      compress_output;

void compress_init(void)
{
    compress_bit_count = 0;
    compress_value = 0;
    compress_bits_in = 0;
    compress_bits_out = 0;
}

bool compress_bit(int bit)
{
//  if (!compress_flag)
//      return (encrypt_bit (bit, inf, outf));

    compress_bits_in++;
    compress_value = (compress_value << 1) | bit;
    if (++compress_bit_count == 8) {
        const char *s = NULL;
        for (s = huffcodes[compress_value]; *s != '\0'; s++) {
//          int bit;

//          if (*s == '1') {
//              bit = 1;
//          } else if (*s == '0') {
//              bit = 0;
//          } else {
//              fprintf (stderr, "Illegal Huffman character '%c'\n", *s);
//              return (FALSE);
//          }
            compress_output.push_back(*s);
    //      if (!encrypt_bit (bit, inf, outf))
    //          return (FALSE);
            compress_bits_out++;
        }

        compress_value = 0;
        compress_bit_count = 0;
    }

    return (true);
}

bool compress_flush(std::string &compress_output_string)
{
    if (compress_bit_count != 0) {
        fprintf (stderr, "Warning: residual of %d bits not compressed\n", compress_bit_count);
    }

    if (compress_bits_out > 0) {
        double cpc = (double)(compress_bits_in - compress_bits_out) / (double) compress_bits_in * 100.0;

        if (cpc < 0.0) {
            fprintf (stderr, "Compression enlarged data by %.2f%% - recommend not using compression\n", -cpc);
        } else {
            fprintf (stderr, "Compressed by %.2f%%\n", cpc);
        }
    }

    compress_output_string = compress_output;
    return true;
}

int compress_character(unsigned char c)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = ((c & (128 >> i)) != 0) ? 1 : 0;
        if (!compress_bit(bit)) {
            return (false);
        }
    }
    return (true);
}

static int  output_bit_count;
static int  output_value;

static void output_init(void)
{
    output_bit_count = 0;
    output_value = 0;
}

static bool output_bit(int bit, std::string &uncompress_out)
{
    output_value = (output_value << 1) | bit;
    if (++output_bit_count == 8) {
        uncompress_out.push_back((char)output_value);
//      outfile_stream << (char)output_value;
//      if (fputc (output_value, outf) == EOF) {
//      perror ("Output file");
//      return (false);
//      }

        output_value = 0;
        output_bit_count = 0;
    }

    return (true);
}

static bool output_flush()
{
    if (output_bit_count > 2) {
        fprintf (stderr, "Warning: residual of %d bits not output\n", output_bit_count);
    }

    return (true);
}

static int  uncompress_bit_count;
static char uncompress_value[256];

void uncompress_init(void)
{
    uncompress_bit_count = 0;
    output_init ();
}

static int huffcode_find(const char  *str)
{
    int i;
    for (i=0; i<256; i++)
        if (strcmp (str, huffcodes[i]) == 0)
        return (i);

    return (-1);
}

bool uncompress_bit(int bit, std::string &uncompress_out)
{
    int code;

//  if (!compress_flag)
//      return (output_bit (bit, outf));

    uncompress_value[uncompress_bit_count++] = bit ? '1' : '0';
    uncompress_value[uncompress_bit_count] = '\0';
    std::cout << "uncompress_value: " << uncompress_value << std::endl;

    if ((code = huffcode_find (uncompress_value)) >= 0) {
        for (int i=0; i<8; i++) {
            int b = ((code & (128 >> i)) != 0) ? 1 : 0;

            if (!output_bit (b, uncompress_out)) {
                return (false);
            }
        }

        uncompress_bit_count = 0;
    }

    if (uncompress_bit_count >= 255) {
        fprintf (stderr, "Error: Huffman uncompress buffer overflow\n");
        return (false);
    }

    return (true);
}

bool uncompress_flush()
{
    if (uncompress_bit_count > 2) {
        fprintf (stderr, "Warning: residual of %d bits not uncompressed\n", uncompress_bit_count);
    }

    return true;
}
