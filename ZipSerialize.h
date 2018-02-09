#pragma once

#include <time.h>

#include <string>
#include <vector>

#include "Exports.h"

#define MAX_MEM_FILE 200 * 1024 * 1024

#define ERR_FILE_EXIST_ZIP      100             // 要添加的文件已经在zip中、重名了

class ZipSerializePrivate;
class ZipSerialize
{
public:
    struct Properties { std::string comment; tm time; unsigned long size; };

    // 压缩方式
    enum TAG_COMPRESS_FLAG_E {
        COMPRESS_FLAG_COMPRESS     = 0,
        COMPRESS_FLAG_DONTCOMPRESS = 1,

        COMPRESS_FLAG_MAX          = 0xFF
    };

public:
    ZipSerialize(const std::string &path, const char *password = nullptr) noexcept;
    ~ZipSerialize() noexcept;
    operator bool() const noexcept;

    std::vector<std::string> list() const;
    int extract(const std::string &file, std::ostream &os) const;
    int addFile(const std::string &containerPath, std::istream &is, const Properties &prop, TAG_COMPRESS_FLAG_E flags = COMPRESS_FLAG_COMPRESS);
    int properties(const std::string &file, Properties &prop) const;
    int save();

    int addFileByPath(const std::string &fileFullPath);
    int addFileListByPath(const std::vector<std::string> &fileFullPathList, const std::string &rootDir = "");
    int extractAllFile(const std::string &dstPath);

private:
    DISABLE_COPY(ZipSerialize);
    ZipSerializePrivate *d;
};
