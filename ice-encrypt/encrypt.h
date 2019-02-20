#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__
#include <string>

struct encrypt_status
{
    ICE_KEY *ice_key = NULL;
    unsigned char encrypt_iv_block[8] = {0};
    std::string encrypt_output;
    std::string decrypt_output;
};
typedef struct encrypt_status ENCRYPT_STATUS_S;

void encrypt_init(ENCRYPT_STATUS_S &encrypt_status, const char *passwd);
int encrypt_bit(ENCRYPT_STATUS_S &encrypt_status, int bit);
void encrypt_flush(ENCRYPT_STATUS_S &encrypt_status, std::string &encrypt_output_string);

void decrypt_init(ENCRYPT_STATUS_S &encrypt_status, const char *passwd);
int decrypt_bit(ENCRYPT_STATUS_S &encrypt_status, int bit);
void decrypt_flush(ENCRYPT_STATUS_S &encrypt_status, std::string &decrypt_output_string);

#endif /* __ENCRYPT_H__ */