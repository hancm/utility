#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <map>

#include "minizip/zip.h"
#include "minizip/unzip.h"

#ifdef _WIN32
#include "minizip/iowin32.h"
#endif

#include "MyLog.h"

#include "File.h"
#include "ZipSerialize.h"

class ZipSerializePrivate
{
public:
    zlib_filefunc64_def pzlib_filefunc;
    std::string path;
    zipFile create;
    unzFile open;
    const char *password;
};

/**
 * Initializes ZIP file serializer.
 *
 * @param path
 */
ZipSerialize::ZipSerialize(const std::string& path, const char *password) noexcept
{
    d = new ZipSerializePrivate;
    if (nullptr == d) {
        return;
    }

#ifdef _WIN32
    fill_win64_filefunc(&d->pzlib_filefunc);
#else
    fill_fopen64_filefunc(&d->pzlib_filefunc);
#endif
    d->path = path;
    d->create = 0;
    d->open = 0;
    d->password = password;

    int append = APPEND_STATUS_CREATE;                          // 默认创建zip方式
    if(MyUtilityLib::File::fileExists(path)) {                  // zip文件已存在
        LOG_DEBUG("Zip file[%s] exist, now add and open zip.", path.c_str());
        append = APPEND_STATUS_ADDINZIP;                        // zip已存在改为添加方式

        // 解压缩zip文件
        d->open = unzOpen2_64((char*)MyUtilityLib::File::encodeName(d->path).c_str(), &d->pzlib_filefunc);
        if(!d->open) {
            LOG_ERROR("Failed to open ZIP file '%s'.", d->path.c_str());
            return;
        }
    }

    // zip文件存在添加、不存在创建
    d->create = zipOpen2_64((char *)MyUtilityLib::File::encodeName(d->path).c_str(), append, 0, &d->pzlib_filefunc);
    if(!d->create) {
        LOG_ERROR("Failed to create ZIP file '%s'.", d->path.c_str());
        return;
    }
}

/**
 * Desctructs ZIP file serializer.
 *
 * @param path
 */
ZipSerialize::~ZipSerialize() noexcept
{
    if(d && d->create) zipClose(d->create, 0);
    if(d && d->open) unzClose(d->open);
    delete d;
    d = nullptr;
}

ZipSerialize::operator bool() const noexcept
{
    return (d && d->create != nullptr) || (d && d->open != nullptr);
}

/**
 * Extracts all files from ZIP file to a temporary directory on disk.
 *
 * @return returns path, where files from ZIP file were extracted.
 * @throws IOException throws exception if there were LOG_ERRORs during
 *         extracting files to disk.
 */
std::vector<std::string> ZipSerialize::list() const
{
    if(!d || !d->open) {
        LOG_ERROR("Zip file is not open");
        return std::vector<std::string>();
    }

    int unzResult = unzGoToFirstFile(d->open);
    std::vector<std::string> list;
    do
    {
        if(unzResult != UNZ_OK) {
            LOG_ERROR("Failed to go to the next file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
            return std::vector<std::string>();
        }

        unz_file_info64 fileInfo;
        unzResult = unzGetCurrentFileInfo64(d->open, &fileInfo, 0, 0, 0, 0, 0, 0);
        if(unzResult != UNZ_OK) {
            LOG_ERROR("Failed to get filename of the current file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
            return std::vector<std::string>();
        }

        std::string fileName(fileInfo.size_filename, 0);
        unzResult = unzGetCurrentFileInfo64(d->open, &fileInfo, &fileName[0], uLong(fileName.size()), 0, 0, 0, 0);
        if(unzResult != UNZ_OK) {
            LOG_ERROR("Failed to get filename of the current file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
            return std::vector<std::string>();
        }

        list.push_back(fileName);
    } while((unzResult = unzGoToNextFile(d->open)) != UNZ_END_OF_LIST_OF_FILE);

    return list;
}

/**
 * Extracts current file from ZIP file to directory pointed in <code>directory</code> parameter.
 *
 * @param zipFile pointer to opened ZIP file.
 * @param directory directory where current file from ZIP should be extracted.
 * @throws IOException throws exception if the extraction of the current file fails from ZIP
 *         file or creating new file to disk failed.
 */
int ZipSerialize::extract(const std::string &file, std::ostream &os) const
{
    LOG_DEBUG("ZipSerializePrivate::extract(%s)", file.c_str());
    if(file.empty() ||  file[file.size()-1] == '/') {
        LOG_ERROR("File[%s] can not empty or end with /.", file.c_str());
        return -1;
    }

    if(!d || !d->open) {
        LOG_ERROR("Zip file is not open");
        return -1;
    }

    int unzResult = unzLocateFile(d->open, file.c_str(), 0);
    if(unzResult != UNZ_OK) {
        LOG_ERROR("Failed to open file[%s] inside ZIP container. ZLib LOG_ERROR: %d", file.c_str(), unzResult);
        return unzResult;
    }

    unzResult = unzOpenCurrentFilePassword(d->open, d->password);
    if(unzResult != UNZ_OK) {
        LOG_ERROR("Failed to open file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
        return unzResult;
    }

    double currentStreamSize = 0;
    char buf[10240];
    for( ;; )
    {
        unzResult = unzReadCurrentFile(d->open, buf, 10240);
        if(unzResult == UNZ_EOF)
            break;
        if(unzResult <= UNZ_EOF)
        {
            unzCloseCurrentFile(d->open);
            LOG_ERROR("Failed to read bytes from current file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
            return -1;
        }
        currentStreamSize += unzResult;

        os.write(buf, unzResult);
        if(os.fail())
        {
            unzCloseCurrentFile(d->open);
            LOG_ERROR("Failed to write file '%s' data to stream. Stream size: %f", file.c_str(), currentStreamSize);
            return -1;
        }
    }

    unzResult = unzCloseCurrentFile(d->open);
    if(unzResult != UNZ_OK) {
        LOG_ERROR("Failed to close current file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
        return unzResult;
    }

    return 0;
}

/**
 * Add new file to ZIP container. The file is actually archived to ZIP container after <code>save()</code>
 * method is called.
 *
 * @param containerPath file path inside ZIP file.
 * @param path full path of the file that should be added to ZIP file.
 * @see create()
 * @see save()
 */
int ZipSerialize::addFile(const std::string& containerPath, std::istream &is, const Properties &prop, TAG_COMPRESS_FLAG_E flags)
{
    if(!d || !d->create) {
        LOG_ERROR("Zip file is not open");
        return -1;
    }

    int iRet = -1;

    // 判断文件是否已存在
    if (NULL != d->open) {
        iRet = unzLocateFile(d->open, containerPath.c_str(), 0);
        if(iRet == UNZ_OK) {
            LOG_ERROR("File[%s] exists.", containerPath.c_str());
            return -1;
        }
    }

    LOG_DEBUG("ZipSerialize::addFile(%s)", containerPath.c_str());
    zip_fileinfo info = {
        { uInt(prop.time.tm_sec), uInt(prop.time.tm_min), uInt(prop.time.tm_hour),
          uInt(prop.time.tm_mday), uInt(prop.time.tm_mon), uInt(prop.time.tm_year) },
        0, 0, 0 };

    // Create new file inside ZIP container.
    // 2048 general purpose bit 11 for unicode
    int compression = flags & COMPRESS_FLAG_DONTCOMPRESS ? Z_NULL : Z_DEFLATED;
    int level = flags & COMPRESS_FLAG_DONTCOMPRESS ? Z_NO_COMPRESSION : Z_DEFAULT_COMPRESSION;
    int zipResult = zipOpenNewFileInZip4(d->create, containerPath.c_str(),
        &info, 0, 0, 0, 0, prop.comment.c_str(), compression, level, 0,
        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, d->password, 0, 0, 2048);
    if(zipResult != ZIP_OK) {
        LOG_ERROR("Failed to create new file inside ZIP container. ZLib LOG_ERROR: %d", zipResult);
        return -1;
    }

    is.clear();
    is.seekg(0);
    char buf[10240];
    while( is )
    {
        is.read(buf, 10240);
        if(is.gcount() <= 0)
            break;

        zipResult = zipWriteInFileInZip(d->create, buf, (unsigned int)is.gcount());
        if(zipResult != ZIP_OK)
        {
            zipCloseFileInZip(d->create);
            LOG_ERROR("Failed to write bytes to current file inside ZIP container. ZLib LOG_ERROR: %d", zipResult);
            return -1;
        }
    }

    zipResult = zipCloseFileInZip(d->create);
    if(zipResult != ZIP_OK) {
        LOG_ERROR("Failed to close current file inside ZIP container. ZLib LOG_ERROR: %d", zipResult);
        return -1;
    }

    return 0;
}

int ZipSerialize::properties(const std::string &file, ZipSerialize::Properties &prop) const
{
    if(!d || !d->open) {
        LOG_ERROR("Zip file is not open");
        return -1;
    }

    int unzResult = unzLocateFile(d->open, file.c_str(), 0);
    if(unzResult != UNZ_OK) {
        LOG_ERROR("Failed to open file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
        return -1;
    }

    unz_file_info64 info;
    unzResult = unzGetCurrentFileInfo64(d->open, &info, 0, 0, 0, 0, 0, 0);
    if(unzResult != UNZ_OK) {
        LOG_ERROR("Failed to get filename of the current file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
        return -1;
    }

    tm time = { int(info.tmu_date.tm_sec), int(info.tmu_date.tm_min), int(info.tmu_date.tm_hour),
            int(info.tmu_date.tm_mday), int(info.tmu_date.tm_mon), int(info.tmu_date.tm_year), 0, 0, 0
#ifndef _WIN32
             , 0, 0
#endif
    };

    prop.time = time;
    prop.size = info.uncompressed_size;

    if(info.size_file_comment == 0) {
        return 0;
    }

    prop.comment.resize(info.size_file_comment);
    unzResult = unzGetCurrentFileInfo64(d->open, &info, 0, 0, 0, 0, &prop.comment[0], uLong(prop.comment.size()));
    if(unzResult != UNZ_OK) {
        LOG_ERROR("Failed to get filename of the current file inside ZIP container. ZLib LOG_ERROR: %d", unzResult);
        return -1;
    }

    return 0;
}

/**
 * Creates new ZIP file and adds all the added files to the ZIP file.
 *
 * @throws IOException throws exception if the file to be added did not exists or creating new
 *         ZIP file failed.
 * @see create()
 * @see addFile(const std::string& containerPath, const std::string& path)
 */
int ZipSerialize::save()
{
    if(!d || !d->create) {
        LOG_ERROR("Zip file is not open");
        return -1;
    }

    LOG_DEBUG("ZipSerialize::close()");
    int zipResult = zipClose(d->create, nullptr);
    d->create = 0;
    if(zipResult != ZIP_OK) {
        LOG_ERROR("Failed to close ZIP file. ZLib LOG_ERROR: %d", zipResult);
        return -1;
    }

    return 0;
}

int ZipSerialize::addFileByPath(const std::string &fileFullPath)
{
    std::string rootDir(fileFullPath);
    std::vector<std::string> fileFullPathList;
    if(MyUtilityLib::File::directoryExists(fileFullPath)) {
        MyUtilityLib::File::listFiles(fileFullPath, fileFullPathList, 1);
    } else {
        fileFullPathList.push_back(fileFullPath);
        rootDir.clear();
    }

    if (fileFullPathList.empty()) {
        LOG_ERROR("File path is empty.");
        return -1;
    }

    int iRet = addFileListByPath(fileFullPathList, rootDir);
    if (0 != iRet) {
        LOG_ERROR("Failed to add file to zip.");
        return iRet;
    }

    return 0;
}

int ZipSerialize::addFileListByPath(const std::vector<std::string> &fileFullPathList, const std::string &rootDir)
{

    //
    // 设置添加zip的文件名
    //
    std::map<std::string, std::string> zipFileNameMap;          // {文件路径，存入zip文件名}
    for (auto fileFullPath : fileFullPathList)
    {
        if(fileFullPath.empty() || !MyUtilityLib::File::fileExists(fileFullPath)) {
            LOG_ERROR("Document file '%s' empty or does not exist.", fileFullPath.c_str());
            return -1;
        }

        std::string zipFileName;
        if (rootDir.empty()) {
            // 文件路径只有文件名称
            zipFileName = MyUtilityLib::File::fileName(fileFullPath);
        } else {
            // 目录带上相对路径
            zipFileName = fileFullPath.substr(rootDir.size());
        }

        // 删除开头的‘/’
        if (!zipFileName.empty() && zipFileName[0] == '/') {
            zipFileName = zipFileName.substr(1);
        }

        zipFileNameMap[fileFullPath] = zipFileName;
    }

    //
    // 判断文件是否已存在zip中
    // 若存在返回失败
    //
    std::vector<std::string> fileList = this->list();
    for(auto file : zipFileNameMap) {
        LOG_DEBUG("file %s.", file.second.c_str());
        if (fileList.end() != std::find(fileList.begin(), fileList.end(), file.second)) {
            // 文件已存在，返回失败
            LOG_ERROR("File[%s] exist in zip.", file.second.c_str());
            return ERR_FILE_EXIST_ZIP;
        }
    }

    //
    // 添加文件到zip
    //
    for (auto fileFullPath : fileFullPathList)
    {
        // zip属性
        tm *filetime = MyUtilityLib::File::modifiedTime(fileFullPath);
        ZipSerialize::Properties prop = { "", *filetime, MyUtilityLib::File::fileSize(fileFullPath) };

        // 输入文件流
        std::ifstream fileStream(MyUtilityLib::File::encodeName(fileFullPath).c_str(), std::ifstream::binary);
        if (!fileStream || !fileStream.is_open()) {
            LOG_ERROR("Failed to open ifstream for path[%s].", fileFullPath.c_str());
            return -1;
        }

        int iRet = -1;
        if(prop.size > MAX_MEM_FILE)
        {
            iRet = addFile(zipFileNameMap[fileFullPath], fileStream, prop, COMPRESS_FLAG_COMPRESS);
        }
        else
        {
            std::stringstream dataStream;
            dataStream << fileStream.rdbuf();;
            iRet = addFile(zipFileNameMap[fileFullPath], dataStream, prop, COMPRESS_FLAG_COMPRESS);
        }

        if (0 != iRet) {
            LOG_ERROR("Failed to add file[%s] to zip.", fileFullPath.c_str());
            return iRet;
        }
    }

    return 0;
}

int ZipSerialize::extractAllFile(const std::string &dstPath)
{
    std::vector<std::string> fileList = list();
    if (fileList.empty()) {
        LOG_ERROR("Failed to get file list.");
        return -1;
    }

    for (auto file : fileList) {
        std::string filePath(dstPath + "/" + MyUtilityLib::File::encodeName(file));
        MyUtilityLib::File::createDirectory(MyUtilityLib::File::directory(filePath));
        std::ofstream ofs(filePath, std::ofstream::binary);
        if (!ofs || !ofs.is_open()) {
            LOG_ERROR("Failed to ofstream file[%s].", filePath.c_str());
            return -1;
        }

        int iRet = extract(file, ofs);
        if (0 != iRet) {
            LOG_ERROR("Failed to extract file: %s.", file.c_str());
            return iRet;
        }
    }

    return 0;
}
