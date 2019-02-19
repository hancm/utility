#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include "encode.h"

const int line_length = 80;

/*
 * Return the next tab position.
 */

static int
tabpos (
    int n
) {
    return ((n + 8) & ~7);
}

/*
 * Write a line of text, adding a newline.
 * Return false if the write fails.
 */

static bool
wsputs (
    char        *buf,
    int         size,
    std::string &encode_out
) {
    buf[size++] = '\r';
    encode_out.append(buf, size);
    return (true);
}


/*
 * Calculate, approximately, how many bits can be stored in the line.
 */
static void
whitespace_storage (
    const char  *buf,
    unsigned long   *n_lo,
    unsigned long   *n_hi
) {
    int n = 0, len = strlen (buf);

    if (len > line_length - 2)
        return;

    if (len / 8 == line_length / 8) {
        *n_hi += 3;
        return;
    }

    if ((len & 7) > 0) {
        *n_hi += 3;
        len = tabpos (len);
    }
    if ((line_length & 7) > 0) {
        *n_hi += 3;
    }

    n = ((line_length - len) / 8) * 3;
    *n_hi += n;
    *n_lo += n;
}

/*
 * Load the encode buffer.
 * If there is no text to read, make it empty.
 */

static void
encode_buffer_load (
    ENCODE_STATUS_S &encode_status
) {
    encode_status.encode_buffer[0] = '\0';
    encode_status.encode_buffer_length = strlen (encode_status.encode_buffer);
    encode_status.encode_buffer_column = 0;
    for (int i=0; encode_status.encode_buffer[i] != '\0'; i++) {
        if (encode_status.encode_buffer[i] == '\t') {
            encode_status.encode_buffer_column = tabpos (encode_status.encode_buffer_column);
        } else {
            encode_status.encode_buffer_column++;
        }
    }

    encode_status.encode_buffer_loaded = true;
    encode_status.encode_needs_tab = false;
}

/*
 * Append whitespace to the loaded buffer, if there is room.
 */

static bool
encode_append_whitespace (
    ENCODE_STATUS_S &encode_status,
    int     nsp
) {
    int col = encode_status.encode_buffer_column;

    if (encode_status.encode_needs_tab) {
        col = tabpos (col);
    }

    if (nsp == 0) {
        col = tabpos (col);
    } else {
        col += nsp;
    }

    if (col >= line_length) {
        return (false);
    }

    if (encode_status.encode_needs_tab) {
        encode_status.encode_buffer[encode_status.encode_buffer_length++] = '\t';
        encode_status.encode_buffer_column = tabpos (encode_status.encode_buffer_column);
    }

    if (nsp == 0) {
        encode_status.encode_buffer[encode_status.encode_buffer_length++] = '\t';
        encode_status.encode_buffer_column = tabpos (encode_status.encode_buffer_column);
        encode_status.encode_needs_tab = false;
    } else {
        int i;
        for (i=0; i<nsp; i++) {
            encode_status.encode_buffer[encode_status.encode_buffer_length++] = ' ';
            encode_status.encode_buffer_column++;
        }
        encode_status.encode_needs_tab = true;
    }

    encode_status.encode_buffer[encode_status.encode_buffer_length] = '\0';
    return (true);
}


/*
 * Write a value into the text.
 */

static bool
encode_write_value (
    ENCODE_STATUS_S &encode_status,
    int val
) {
    if (!encode_status.encode_buffer_loaded) {
        encode_buffer_load (encode_status);
    }

    // 加密空白以Tab开头
    if (!encode_status.encode_first_tab) {                                                /* Tab shows start of data */
        while (tabpos (encode_status.encode_buffer_column) >= line_length) {
            if (!wsputs (encode_status.encode_buffer, strlen(encode_status.encode_buffer), encode_status.encode_out)) {
                return (false);
            }
            encode_buffer_load (encode_status);
        }

        encode_status.encode_buffer[encode_status.encode_buffer_length++] = '\t';
        encode_status.encode_buffer[encode_status.encode_buffer_length] = '\0';
        encode_status.encode_buffer_column = tabpos (encode_status.encode_buffer_column);
        encode_status.encode_first_tab = true;
    }

    /* Reverse the bit ordering */
    int nspc = ((val & 1) << 2) | (val & 2) | ((val & 4) >> 2);

    while (!encode_append_whitespace (encode_status, nspc)) {
        if (!wsputs (encode_status.encode_buffer, strlen(encode_status.encode_buffer), encode_status.encode_out)) {
            return (false);
        }
        encode_buffer_load (encode_status);
    }

    return (true);
}


/*
 * Flush the rest of the text to the output.
 */

static bool
encode_write_flush (
    ENCODE_STATUS_S &encode_status
) {
    unsigned long n_lo = 0;
    unsigned long n_hi = 0;

    if (encode_status.encode_buffer_loaded) {
        if (!wsputs(encode_status.encode_buffer, strlen(encode_status.encode_buffer), encode_status.encode_out)) {
            return (false);
        }
        encode_status.encode_buffer_loaded = false;
        encode_status.encode_buffer_length = 0;
        encode_status.encode_buffer_column = 0;
    }

    return (true);
}


/*
 * Initialize the encoding routines.
 */

void encode_init(ENCODE_STATUS_S &encode_status)
{
    encode_status = ENCODE_STATUS_S();
}


/*
 * Encode a single bit.
 */

bool
encode_bit (
    ENCODE_STATUS_S &encode_status,
    int     bit
) {
    encode_status.encode_value = (encode_status.encode_value << 1) | bit;
    if (++encode_status.encode_bit_count == 3) {
        if (!encode_write_value (encode_status, encode_status.encode_value)) {
            return (false);
        }

        encode_status.encode_value = 0;
        encode_status.encode_bit_count = 0;
    }

    return (true);
}


/*
 * Flush the contents of the encoding routines.
 */

bool
encode_flush (
    ENCODE_STATUS_S &encode_status,
    std::string &encode_output
) {
    if (encode_status.encode_bit_count > 0) {

        // 3 bits进行数据对齐
        while (encode_status.encode_bit_count < 3) {  /* Pad to 3 bits */
            encode_status.encode_value <<= 1;
            encode_status.encode_bit_count++;
        }

        if (!encode_write_value (encode_status, encode_status.encode_value)) {
            return (false);
        }
    }

    if (!encode_write_flush (encode_status)) {
        return (false);
    }

    encode_output = encode_status.encode_out;

    return (true);
}

static int
character_encode(ENCODE_STATUS_S &encode_status,
                 unsigned char c)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = ((c & (128 >> i)) != 0) ? 1 : 0;
        if (!encode_bit (encode_status, bit)) {
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
message_string_encode(const std::string &encode_message, std::string &encode_output)
{
    ENCODE_STATUS_S encode_status;
    encode_init(encode_status);

    for (size_t i = 0; i < encode_message.size(); ++i) {
        if (!character_encode (encode_status, encode_message[i])) {
            return -1;
        }
    }

    if (!encode_flush(encode_status, encode_output)) {
        return -1;
    }

    return 0;
}

static int  output_bit_count = 0;
static int  output_value = 0;

static bool
output_bit (
    int     bit,
    std::string &encode_output
) {
    output_value = (output_value << 1) | bit;
    if (++output_bit_count == 8) {
        encode_output.push_back((char)output_value);
        output_value = 0;
        output_bit_count = 0;
    }

    return (true);
}


/*
 * Decode the space count into actual bits.
 */

static bool
decode_bits (
    int     spc,
    std::string &encode_output
) {
    int b1 = 0, b2 = 0, b3 = 0;

    if (spc > 7) {
        fprintf (stderr, "Illegal encoding of %d spaces\n", spc);
        return (false);
    }

    if ((spc & 1) != 0) {
        b1 = 1;
    }
    if ((spc & 2) != 0) {
        b2 = 1;
    }
    if ((spc & 4) != 0) {
        b3 = 1;
    }

    if (!output_bit (b1, encode_output)) {
        return (false);
    }
    if (!output_bit (b2, encode_output)) {
        return (false);
    }
    if (!output_bit (b3, encode_output)) {
        return (false);
    }

    return (true);
}


/*
 * Decode the whitespace contained in the string.
 */

static bool
decode_whitespace (
    const char  *s,
    std::string &encode_output
) {
    int spc = 0;
    for (;; s++) {
        if (*s == ' ') {
            spc++;
        } else if (*s == '\t') {
            if (!decode_bits (spc, encode_output)) {
                return (false);
            }
            spc = 0;
        } else if (*s == '\0') {
            if (spc > 0 && !decode_bits (spc, encode_output)) {
                return (false);
            }
            return (true);
        }
    }
}

/*
 * Extract a message from the input stream.
 */

int
message_extract(const std::string &encode_string_info,
                std::string &encode_output)
{
    // 获取数据
    std::istringstream infile_stream(encode_string_info);
    std::vector<std::string> vecDecodeString;
    std::string strSplite;
    while (std::getline(infile_stream, strSplite, '\r')) {
        vecDecodeString.push_back(strSplite);
    }

    if (vecDecodeString.empty()) {
        return -1;
    }

    /**
     * 解码相应数据
     */
    bool start_tab_found = false;
    for (size_t i = 0; i < vecDecodeString.size(); ++i) {
        std::string &strDecodeTmp = vecDecodeString[i];

        char *s = NULL;
        char *last_ws = NULL;
        for (s = (char*)strDecodeTmp.c_str(); *s != '\0' && *s != '\n' && *s != '\r'; s++) {
            if (*s != ' ' && *s != '\t') {
                last_ws = NULL;
            } else if (last_ws == NULL) {
                last_ws = s;
            }
        }

        if (*s == '\n' || *s == '\r') {
            *s = '\0';
        }

        if (last_ws == NULL) {
            continue;
        }

        if (!start_tab_found && *last_ws == ' ') {
            continue;
        }

        if (!start_tab_found && *last_ws == '\t') {
            start_tab_found = true;
            last_ws++;
            if (*last_ws == '\0') {
                continue;
            }
        }

        if (!decode_whitespace (last_ws, encode_output)) {
            fprintf(stderr, "Failed to decode whitespace.\n");
            return -1;
        }
    }

    return 0;
}


/*
 * Calculate the amount of covert information that can be stored
 * in the file.
 */

//void
//space_calculate (
//    FILE        *fp
//) {
//    unsigned long   n_lo = 0, n_hi = 0;
//    char buf[1024 * 1024 * 2] = {0};
//
//    while (wsgets (buf, sizeof(buf), fp) != NULL) {
//        whitespace_storage (buf, &n_lo, &n_hi);
//    }
//
//    if (n_lo > 0) {     /* Allow for initial tab */
//        n_lo--;
//        n_hi--;
//    }
//
//    if (n_lo == n_hi) {
//        printf ("File has storage capacity of %ld bits (%ld bytes)\n",
//                            n_lo, n_lo / 8);
//    } else {
//        printf ("File has storage capacity of between %ld and %ld bits.\n",
//                                n_lo, n_hi);
//        printf ("Approximately %ld bytes.\n", (n_lo + n_hi) / 16);
//    }
//}
