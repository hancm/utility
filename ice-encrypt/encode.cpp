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
 * Local variables used for encoding.
 */

//static int      encode_status.encode_bit_count;                   // 编码bit数目，等于3一次编码
//static int      encode_status.encode_value;                       // 编码值
//static char     encode_status.encode_buffer[1024 * 1024 * 2];     // 编码缓存，没有结尾空白符
//static bool     encode_status.encode_buffer_loaded;               // 编码缓存是否已加载，encode_buffer_load后加载
//static int      encode_status.encode_buffer_length;               // 编码缓存长度(字节数)
//static int      encode_status.encode_buffer_column;               // 缓存列长度(tab算4个，一个字符一个)
//static bool     encode_status.encode_first_tab;
//static bool     encode_status.encode_needs_tab;                   // 编码是否需要tab
//static unsigned long    encode_status.encode_bits_used;
//static unsigned long    encode_status.encode_bits_available;
//static unsigned long    encode_status.encode_lines_extra;         // 行后没有编码完的，需要额外行数目


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
 * Read a line of text, like fgets, but strip off trailing whitespace.
 */

static void wsremove(char *buf)
{
    int n = strlen (buf) - 1;
    while (n >= 0 && (buf[n] == ' ' || buf[n] == '\t' ||
                      buf[n] == '\n' || buf[n] == '\r')) {
        buf[n] = '\0';
        n--;
    }
}


static char *
wsgets (
    char        *buf,
    int         size,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream,
    char        *filter_word_string = "%%EOF%%EOF",
    int         text_encode_mode = 1,
    char        splite_char = '\r'
) {
    return NULL;

//  // 指定单词模式
//  int flag = 0;
//  std::string strSplite;
//  if (1 == text_encode_mode || 2 == text_encode_mode) {
//      while (infile_stream.good()) {
//          strSplite.clear();
//          std::getline(infile_stream, strSplite, splite_char);
//          std::string strSource;
//          strSource = strSplite + splite_char;              // getline不会读取最后的换行符
//
//          if (strSplite != filter_word_string) {
//              // 没找到需要的字符串
//              // 把原字符串输出到文件
//              if (!outfile_stream.write(strSource.c_str(), strSource.size())) {
//                  fprintf(stderr, "Failed to write %s to outfile.", strSource.c_str());
//                  return NULL;
//              }
//              continue;
//          }
//
//          /**
//           * 目前设计pdf走不到这里
//           */
//          fprintf(stderr, "find@: ");
//          for (size_t i = 0; i < strSource.size(); ++i) {
//              if ('\r' == strSource[i]) {
//                  fprintf(stderr, "\\r");
//              } else if ('\n' == strSource[i]) {
//                  fprintf(stderr, "\\n");
//              } else {
//                  fprintf(stderr, "%c", strSource[i]);
//              }
//          }
//          fprintf(stderr, "\n");
//
//          memset(buf, 0, size);
//          strncpy(buf, filter_word_string, size - 1);
//          wsremove(buf);
//          flag = 1;
//          break;
//      }
//  } else {
//      std::getline(infile_stream, strSplite);
//      memset(buf, 0, size);
//      strncpy(buf, strSplite.c_str(), size - 1);
//      wsremove(buf);
//      flag = 1;
//  }
//
//  return flag ? buf : NULL;
}


/*
 * Write a line of text, adding a newline.
 * Return false if the write fails.
 */

static bool
wsputs (
    char        *buf,
    int         size,
    std::ostringstream &outfile_stream,
    char    splite_char = '\r'
) {
    buf[size++] = splite_char;
    if (!outfile_stream.write(buf, size)) {
        fprintf(stderr, "Error: failed to wsputs write.\n");
        return (false);
    }

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
    int     n, len = strlen (buf);

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
    ENCODE_STATUS_S &encode_status,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream
) {
    if (wsgets (encode_status.encode_buffer, sizeof(encode_status.encode_buffer), infile_stream, outfile_stream) == NULL) {
        encode_status.encode_buffer[0] = '\0';
        encode_status.encode_lines_extra++;
    }

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
    int     val,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream
) {
    if (!encode_status.encode_buffer_loaded) {
        encode_buffer_load (encode_status, infile_stream, outfile_stream);
    }

    // 加密空白以Tab开头
    if (!encode_status.encode_first_tab) {                                                /* Tab shows start of data */
        while (tabpos (encode_status.encode_buffer_column) >= line_length) {
            if (!wsputs (encode_status.encode_buffer, strlen(encode_status.encode_buffer), outfile_stream)) {
                return (false);
            }
            encode_buffer_load (encode_status, infile_stream, outfile_stream);
        }

        encode_status.encode_buffer[encode_status.encode_buffer_length++] = '\t';
        encode_status.encode_buffer[encode_status.encode_buffer_length] = '\0';
        encode_status.encode_buffer_column = tabpos (encode_status.encode_buffer_column);
        encode_status.encode_first_tab = true;
    }

    /* Reverse the bit ordering */
    int nspc = ((val & 1) << 2) | (val & 2) | ((val & 4) >> 2);

    while (!encode_append_whitespace (encode_status, nspc)) {
        if (!wsputs (encode_status.encode_buffer, strlen(encode_status.encode_buffer), outfile_stream)) {
            return (false);
        }
        encode_buffer_load (encode_status, infile_stream, outfile_stream);
    }

    if (encode_status.encode_lines_extra == 0) {
        encode_status.encode_bits_available += 3;
    }

    return (true);
}


/*
 * Flush the rest of the text to the output.
 */

static bool
encode_write_flush (
    ENCODE_STATUS_S &encode_status,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream,
    char splite_char = '\r'
) {
    unsigned long n_lo = 0;
    unsigned long n_hi = 0;

    if (encode_status.encode_buffer_loaded) {
        if (!wsputs(encode_status.encode_buffer, strlen(encode_status.encode_buffer), outfile_stream)) {
            return (false);
        }
        encode_status.encode_buffer_loaded = false;
        encode_status.encode_buffer_length = 0;
        encode_status.encode_buffer_column = 0;
    }

//  std::string str;
//  while (std::getline(infile_stream, str, splite_char)) {
//      str += splite_char;
//      whitespace_storage (str.c_str(), &n_lo, &n_hi);
//
//      if (!outfile_stream.write(/*tmp_buf, infile_stream.gcount()*/str.c_str(), str.size()))
//          return (false);
//  }

    encode_status.encode_bits_available += (n_lo + n_hi) / 2;

    return (true);
}


/*
 * Initialize the encoding routines.
 */

void encode_init(ENCODE_STATUS_S &encode_status)
{
//  encode_status.encode_bit_count = 0;
//  encode_status.encode_value = 0;
//  encode_status.encode_buffer_loaded = false;
//  encode_status.encode_buffer_length = 0;
//  encode_status.encode_buffer_column = 0;
//  encode_status.encode_first_tab = false;
//  encode_status.encode_bits_used = 0;
//  encode_status.encode_bits_available = 0;
//  encode_status.encode_lines_extra = 0;
//  encode_status.encode_needs_tab = false;

    memset(&encode_status, 0, sizeof(encode_status));
}


/*
 * Encode a single bit.
 */

bool
encode_bit (
    ENCODE_STATUS_S &encode_status,
    int     bit,
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream
) {
    encode_status.encode_value = (encode_status.encode_value << 1) | bit;
    encode_status.encode_bits_used++;

    if (++encode_status.encode_bit_count == 3) {
        if (!encode_write_value (encode_status, encode_status.encode_value, infile_stream, outfile_stream)) {
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
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream,
    std::string &encodeBuffer
) {
    if (encode_status.encode_bit_count > 0) {

        // 3 bits进行数据对齐
        while (encode_status.encode_bit_count < 3) {  /* Pad to 3 bits */
            encode_status.encode_value <<= 1;
            encode_status.encode_bit_count++;
        }

        if (!encode_write_value (encode_status, encode_status.encode_value, infile_stream, outfile_stream)) {
            return (false);
        }
    }

    if (!encode_write_flush (encode_status, infile_stream, outfile_stream)) {
        return (false);
    }

    encodeBuffer = encode_status.encode_buffer;

//  if (!quiet_flag) {
//      if (encode_status.encode_lines_extra > 0) {
//          fprintf (stderr, "Message exceeded available space by approximately %.2f%%.\n",
//                   ((double) encode_status.encode_bits_used / encode_status.encode_bits_available - 1.0) * 100.0);
//
//          fprintf (stderr, "An extra %ld lines were added.\n", encode_status.encode_lines_extra);
//      } else {
//          fprintf (stderr, "Message used approximately %.2f%% of available space.\n",
//               (double) encode_status.encode_bits_used / encode_status.encode_bits_available * 100.0);
//      }
//  }

    return (true);
}

static int
character_encode(ENCODE_STATUS_S &encode_status,
                 unsigned char c,
                 std::istringstream &infile_stream,
                 std::ostringstream &outfile_stream)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = ((c & (128 >> i)) != 0) ? 1 : 0;
        if (!encode_bit (encode_status, bit, infile_stream, outfile_stream)) {
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
message_string_encode(ENCODE_STATUS_S &encode_status,
                      const char *msg,
                      std::istringstream &infile_stream,
                      std::ostringstream &outfile_stream)
{
    while (*msg != '\0') {
        if (!character_encode (encode_status, *msg, infile_stream, outfile_stream)) {
            return (false);
        }
        msg++;
    }

    return 0;
}

static int  output_bit_count = 0;
static int  output_value = 0;

static bool
output_bit (
    int     bit,
    std::ostringstream &outfile_stream
) {
    output_value = (output_value << 1) | bit;
    std::cout << "output value: " << output_value << std::endl;
    if (++output_bit_count == 8) {
        std::cout << "output value2: " << output_value << std::endl;
        outfile_stream << (char)output_value;
//      if (fputc (output_value, outf) == EOF) {
//          perror ("Output file");
//          return (false);
//      }

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
    std::ostringstream &outfile_stream
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

    if (!output_bit (b1, outfile_stream)) {
        return (false);
    }
    if (!output_bit (b2, outfile_stream)) {
        return (false);
    }
    if (!output_bit (b3, outfile_stream)) {
        return (false);
    }

//  if (!decrypt_bit (b1, outf)) {
//      return (false);
//  }
//  if (!decrypt_bit (b2, outf)) {
//      return (false);
//  }
//  if (!decrypt_bit (b3, outf)) {
//      return (false);
//  }

    return (true);
}


/*
 * Decode the whitespace contained in the string.
 */

static bool
decode_whitespace (
    const char  *s,
    std::ostringstream &outfile_stream
) {
    int     spc = 0;
    std::cout << "s: " << s << std::endl;
    for (;; s++) {
        if (*s == ' ') {
            spc++;
        } else if (*s == '\t') {
            if (!decode_bits (spc, outfile_stream)) {
                return (false);
            }
            spc = 0;
        } else if (*s == '\0') {
            if (spc > 0 && !decode_bits (spc, outfile_stream)) {
                return (false);
            }
            return (true);
        }
    }
}

/*
 * Extract a message from the input stream.
 */

bool
message_extract (
    std::istringstream &infile_stream,
    std::ostringstream &outfile_stream,
    int     text_encode_mode,
    char splite_char
) {
    bool start_tab_found = false;

    //decrypt_init ();

    /**
     * 先找到所有需要解码的数据
     */
    std::vector<std::string> vecDecodeString;
    std::string strSplite;
    bool bFindEOF = false;
    while (std::getline(infile_stream, strSplite, splite_char)) {
        std::cout << "Splite size: " << strSplite.size() << std::endl;
//      if (1== text_encode_mode) {
//          // PDF模式添加密文到文件末尾
//
//          // 上次查询找到了EOF
//          if (bFindEOF) {
//              if (std::string::npos != strSplite.find_first_not_of(" \t\n\r")) {
//                  // 出现字符，找到结尾
//                  bFindEOF = false;
//                  continue;
//              } else if (std::string::npos != strSplite.find("\t")) {
//                  vecDecodeString.push_back(strSplite);
//              }
//          }
//
//          if (std::string::npos != strSplite.find("%%EOF")) {
//              bFindEOF = true;
//          }
//      } else {
            // 普通text模式
            vecDecodeString.push_back(strSplite);
//      }
    }

    /**
     * 解码相应数据
     */
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

        if (!decode_whitespace (last_ws, outfile_stream)) {
            fprintf(stderr, "Failed to decode whitespace.\n");
            return (false);
        }
    }

    return true;//(decrypt_flush (outf));
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
