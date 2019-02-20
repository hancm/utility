#include <stdio.h>
#include <string.h>
#include <iostream>

#include "ice.h"
#include "encrypt.h"

static void password_set(ENCRYPT_STATUS_S &encrypt_status, const char *passwd)
{
    int level = (strlen (passwd) * 7 + 63) / 64;
    if (level == 0) {
        fprintf(stderr, "Warning: an empty password is being used\n");
        level = 1;
    } else if (level > 128) {
        fprintf (stderr, "Warning: password truncated to 1170 chars\n");
        level = 128;
    }

    if ((encrypt_status.ice_key = ice_key_create(level)) == NULL) {
        fprintf (stderr, "Warning: failed to set password\n");
        return;
    }

    unsigned char buf[1024] = {0};
    int i = 0;
    while (*passwd != '\0') {
        unsigned char c = *passwd & 0x7f;
        int idx = i / 8;
        int bit = i & 7;

        if (bit == 0) {
            buf[idx] = (c << 1);
        } else if (bit == 1) {
            buf[idx] |= c;
        } else {
            buf[idx] |= (c >> (bit - 1));
            buf[idx + 1] = (c << (9 - bit));
        }

        i += 7;
        passwd++;

        if (i > 8184) {
            break;
        }
    }

    ice_key_set (encrypt_status.ice_key, buf);

    /* Set the initialization vector with the key
     * with itself.
     */
    ice_key_encrypt (encrypt_status.ice_key, buf, encrypt_status.encrypt_iv_block);
}

void encrypt_init(ENCRYPT_STATUS_S &encrypt_status, const char *passwd)
{
    password_set(encrypt_status, passwd);
}

int encrypt_bit(ENCRYPT_STATUS_S &encrypt_status, int bit)
{
    unsigned char buf[8] = {0};
    ice_key_encrypt(encrypt_status.ice_key, encrypt_status.encrypt_iv_block, buf);
    if ((buf[0] & 128) != 0) {
        bit = !bit;
    }

    /* Rotate the IV block one bit left */
    for (int i=0; i<8; i++) {
        encrypt_status.encrypt_iv_block[i] <<= 1;
        if (i < 7 && (encrypt_status.encrypt_iv_block[i+1] & 128) != 0) {
            encrypt_status.encrypt_iv_block[i] |= 1;
        }
    }
    encrypt_status.encrypt_iv_block[7] |= bit;

    char c;
    if (0 == bit) {
        c = '0';
    } else if (1 == bit) {
        c = '1';
    } else {
        return -1;
    }
    encrypt_status.encrypt_output.push_back(c);

    return 0;//(encode_bit (bit, inf, outf));
}

void encrypt_flush(ENCRYPT_STATUS_S &encrypt_status, std::string &encrypt_output_string)
{
    encrypt_output_string = encrypt_status.encrypt_output;
    ice_key_destroy(encrypt_status.ice_key);
}

void decrypt_init(ENCRYPT_STATUS_S &encrypt_status, const char *passwd)
{
    password_set(encrypt_status, passwd);
}

int decrypt_bit(ENCRYPT_STATUS_S &encrypt_status, int bit)
{
    unsigned char buf[8] = {0};
    ice_key_encrypt(encrypt_status.ice_key, encrypt_status.encrypt_iv_block, buf);

    int nbit = 0;
    if ((buf[0] & 128) != 0) {
        nbit = !bit;
    } else {
        nbit = bit;
    }

    /* Rotate the IV block one bit left */
    for (int i=0; i<8; i++) {
        encrypt_status.encrypt_iv_block[i] <<= 1;
        if (i < 7 && (encrypt_status.encrypt_iv_block[i+1] & 128) != 0) {
            encrypt_status.encrypt_iv_block[i] |= 1;
        }
    }
    encrypt_status.encrypt_iv_block[7] |= bit;

    char c;
    if (0 == nbit) {
        c = '0';
    } else if (1 == nbit) {
        c = '1';
    } else {
        return -1;
    }
    encrypt_status.decrypt_output.push_back(c);
    return 0;
}

void decrypt_flush(ENCRYPT_STATUS_S &encrypt_status, std::string &decrypt_output_string)
{
    decrypt_output_string = encrypt_status.decrypt_output;
    ice_key_destroy(encrypt_status.ice_key);
}
