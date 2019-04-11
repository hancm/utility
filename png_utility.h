#ifndef __PNG_H__
#define __PNG_H__

/**
 * @brief 获取PNG图片的宽度和高度
 * @param [IN] imageData    png缓存
 * @param [IN] dataSize     png缓存长度
 * @param [IN] width        png宽度
 * @param [IN] height       png高度
 * @return int
 * @note
 */
int ReadPNGFromBuffer(const unsigned char *imageData, size_t dataSize, int &width, int &height);

#endif /* __PNG_H__ */