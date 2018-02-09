#pragma once

#include <stack>
#include <string>
#include <vector>

#ifdef _WIN32
typedef std::wstring f_string;
#else
typedef std::string f_string;
#endif

namespace MyUtilityLib
{
	class File
	{
	  public:
		  static std::string cwd();
		  static std::string env(const std::string &varname);
		  static bool fileExists(const std::string& path);
		  static bool directoryExists(const std::string& path);
		  static f_string encodeName(const std::string &fileName);
		  static std::string decodeName(const f_string &localFileName);
		  static bool isRelative(const std::string &path);
		  static tm* modifiedTime(const std::string &path);
		  static std::string fileExtension(const std::string &path);
		  static unsigned long fileSize(const std::string &path);
		  static std::string fileName(const std::string& path);
		  static std::string directory(const std::string& path);
		  static std::string path(const std::string& directory, const std::string& relativePath);
		  static std::string fullPathUrl(const std::string &path);
		  static std::string tempFileName();
          static int createDirectory(const std::string& path);
          static int listFiles(const std::string& directory, std::vector<std::string> &files, bool bRecurison = 0);
		  static void deleteTempFiles();
		  static bool removeFile(const std::string &path);
		  static std::string toUri(const std::string &path);
		  static std::string toUriPath(const std::string &path);
		  static std::string fromUriPath(const std::string &path);
		  static std::vector<unsigned char> hexToBin(const std::string &in);
#ifdef __APPLE__
		  static std::string frameworkResourcesPath(const std::string &name);
#endif
#ifdef _WIN32
		  static std::string dllPath(const std::string &dll);
#endif

	private:
#if !defined(_WIN32) && !defined(__APPLE__)
		  static std::string convertUTF8(const std::string &str_in, bool to_UTF);
#endif
		  static std::stack<std::string> tempFiles;
	};
} /* end for namespace MyUtilityLib */

