#ifndef __COMPRESS_H__
#define __COMPRESS_H__

typedef struct compress_status
{
    int           compress_bit_count = 0;
    int           compress_value = 0;
    unsigned long compress_bits_in = 0;
    unsigned long compress_bits_out = 0;
    std::string   compress_output;

    // 解压
    int uncompress_bit_count = 0;
    char uncompress_value[256] = {0};
    int output_bit_count = 0;
    int output_value = 0;
    std::string uncompress_out;
} COMPRESS_STATUS_S;

void compress_init(COMPRESS_STATUS_S &compress_status);
bool compress_bit(COMPRESS_STATUS_S &compress_status, int bit);
bool compress_flush(COMPRESS_STATUS_S &compress_status, std::string &compress_output_string);

int compress_string(const std::string &plain_string, std::string &compress_output_string);

void uncompress_init(COMPRESS_STATUS_S &compress_status);
bool uncompress_bit(COMPRESS_STATUS_S &compress_status, int bit);
bool uncompress_flush(COMPRESS_STATUS_S &compress_status, std::string &uncompress_output_string);

#endif /* __COMPRESS_H__ */