#ifndef __PNG_H__
#define __PNG_H__

int ReadPNGFromBuffer(const unsigned char *imageData, size_t dataSize, int &width, int &height);

#endif /* __PNG_H__ */