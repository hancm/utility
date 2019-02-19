#ifndef __COMPRESS_H__
#define __COMPRESS_H__

typedef struct compress_status
{
    int           compress_bit_count = 0;
    int           compress_value = 0;
    unsigned long compress_bits_in = 0;
    unsigned long compress_bits_out = 0;
    std::string   compress_output;
} COMPRESS_STATUS_S;

void compress_init(COMPRESS_STATUS_S &compress_status);
bool compress_bit(COMPRESS_STATUS_S &compress_status, int bit);
bool compress_flush(COMPRESS_STATUS_S &compress_status, std::string &compress_output_string);

int compress_string(const std::string &plain_string, std::string &compress_output_string);

void uncompress_init(void);
bool uncompress_bit(int bit, std::string &uncompress_out);
bool uncompress_flush();

#endif /* __COMPRESS_H__ */