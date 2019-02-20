#ifndef __ENCODE_H__
#define __ENCODE_H__

typedef struct encode_status
{
    int encode_bit_count = 0;                       // 编码bit数目，等于3一次编码
    int encode_value = 0;                           // 编码值
    char encode_buffer[1024 * 1024 * 2] = {0};      // 编码缓存，没有结尾空白符
    bool encode_buffer_loaded = false;              // 编码缓存是否已加载，encode_buffer_load后加载
    int encode_buffer_length = 0;                   // 编码缓存长度(字节数)
    int encode_buffer_column = 0;                   // 缓存列长度(tab算4个，一个字符一个)
    bool encode_first_tab = false;
    bool encode_needs_tab = false;                  // 编码是否需要tab
    std::string encode_out;                         // 空格编码输出
} ENCODE_STATUS_S;

void encode_init(ENCODE_STATUS_S &encode_status);
bool encode_bit(ENCODE_STATUS_S &encode_status, int bit);
bool encode_flush(ENCODE_STATUS_S &encode_status, std::string &encode_output);

int message_string_encode(const std::string &encode_message, std::string &encode_output);
int message_extract(const std::string &encode_string_info, std::string &encode_message);

#endif /* __ENCODE_H__ */