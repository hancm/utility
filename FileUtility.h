#pragma once

#include <ctime>
#include <string>
#include <vector>

class FileUtility
{
  public:
      static std::string cwd();
      static std::string env(const std::string &varname);
      static bool fileExists(const std::string &path);
      static bool directoryExists(const std::string &path);
      static bool isRelative(const std::string &path);
      static tm *modifiedTime(const std::string &path);
      static std::string fileExtension(const std::string &path);
      static size_t fileSize(const std::string &path);
      static std::string fileName(const std::string &path);
      static std::string directory(const std::string &path);
      static std::string path(const std::string &directory, const std::string &relativePath);
      static std::string fullPathUrl(const std::string &path);
      static std::string tempFileName();
      static int createDirectory(const std::string &path);
      static int listFiles(const std::string &directory, std::vector<std::string> &files, bool bRecurison = 0);
      static bool removeFile(const std::string &path);
      static std::string toUri(const std::string &path);
      static std::string toUriPath(const std::string &path);
      static std::string fromUriPath(const std::string &path);
      static std::vector<unsigned char> hexToBin(const std::string &in);
      static int readFileInfo(const std::string &strFilePath, std::string &vecFileBuf);
      static int writeFileInfo(const std::string &strFilePath, const char *fileBuf, size_t fileLen);
      static int copyFile(const char *srcFilePath, const char *dstFilePath);
};

