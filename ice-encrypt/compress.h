#ifndef __COMPRESS_H__
#define __COMPRESS_H__

void compress_init(void);
bool compress_bit(int bit);
int compress_character(unsigned char c);
bool compress_flush(std::string &compress_output_string);

void uncompress_init(void);
bool uncompress_bit(int bit, std::string &uncompress_out);
bool uncompress_flush();

#endif /* __COMPRESS_H__ */