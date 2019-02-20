#include <string.h>
#include <string>
#include <iostream>
#include "compress.h"

static const char *huffcodes[256] = {
#include "huffcode.h"
};

void compress_init(COMPRESS_STATUS_S &compress_status)
{
    compress_status = COMPRESS_STATUS_S();
}

bool compress_bit(COMPRESS_STATUS_S &compress_status, int bit)
{
    compress_status.compress_bits_in++;
    compress_status.compress_value = (compress_status.compress_value << 1) | bit;
    if (++compress_status.compress_bit_count == 8) {
        const char *s = NULL;
        for (s = huffcodes[compress_status.compress_value]; *s != '\0'; s++) {
            compress_status.compress_output.push_back(*s);
            compress_status.compress_bits_out++;
        }

        compress_status.compress_value = 0;
        compress_status.compress_bit_count = 0;
    }

    return (true);
}

bool compress_flush(COMPRESS_STATUS_S &compress_status, std::string &compress_output_string)
{
    if (compress_status.compress_bit_count != 0) {
        fprintf (stderr, "Warning: residual of %d bits not compressed\n", compress_status.compress_bit_count);
    }

    if (compress_status.compress_bits_out > 0) {
        double cpc = (double)(compress_status.compress_bits_in - compress_status.compress_bits_out) / (double)compress_status.compress_bits_in * 100.0;

        if (cpc < 0.0) {
            fprintf (stderr, "Compression enlarged data by %.2f%% - recommend not using compression\n", -cpc);
        } else {
            fprintf (stderr, "Compressed by %.2f%%\n", cpc);
        }
    }

    compress_output_string = compress_status.compress_output;
    return true;
}

static int compress_character(COMPRESS_STATUS_S &compress_status, unsigned char c)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = ((c & (128 >> i)) != 0) ? 1 : 0;
        if (!compress_bit(compress_status, bit)) {
            return (false);
        }
    }
    return (true);
}

int compress_string(const std::string &plain_string, std::string &compress_output_string)
{
    COMPRESS_STATUS_S compress_status;
    compress_init(compress_status);

    for (size_t i = 0; i < plain_string.size(); ++i) {
        compress_character(compress_status, plain_string[i]);
    }

    compress_flush(compress_status, compress_output_string);
    return 0;
}

static bool output_bit(COMPRESS_STATUS_S &compress_status, int bit)
{
    compress_status.output_value = (compress_status.output_value << 1) | bit;
    if (++compress_status.output_bit_count == 8) {
        compress_status.uncompress_out.push_back((char)compress_status.output_value);
        compress_status.output_value = 0;
        compress_status.output_bit_count = 0;
    }

    return (true);
}

void uncompress_init(COMPRESS_STATUS_S &compress_status)
{
    compress_status = COMPRESS_STATUS_S();
}

static int huffcode_find(const char  *str)
{
    int i;
    for (i=0; i<256; i++)
        if (strcmp (str, huffcodes[i]) == 0)
        return (i);

    return (-1);
}

bool uncompress_bit(COMPRESS_STATUS_S &compress_status, int bit)
{
    compress_status.uncompress_value[compress_status.uncompress_bit_count++] = bit ? '1' : '0';
    compress_status.uncompress_value[compress_status.uncompress_bit_count] = '\0';
    int code = 0;
    if ((code = huffcode_find (compress_status.uncompress_value)) >= 0) {
        for (int i=0; i<8; i++) {
            int b = ((code & (128 >> i)) != 0) ? 1 : 0;

            if (!output_bit(compress_status, b)) {
                return (false);
            }
        }

        compress_status.uncompress_bit_count = 0;
    }

    if (compress_status.uncompress_bit_count >= 255) {
        fprintf (stderr, "Error: Huffman uncompress buffer overflow\n");
        return (false);
    }

    return (true);
}

bool uncompress_flush(COMPRESS_STATUS_S &compress_status, std::string &uncompress_output_string)
{
    if (compress_status.uncompress_bit_count > 2) {
        fprintf (stderr, "Warning: residual of %d bits not uncompressed\n", compress_status.uncompress_bit_count);
    }
    uncompress_output_string = compress_status.uncompress_out;

    return true;
}
