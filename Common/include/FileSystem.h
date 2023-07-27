#pragma once
#include <sys/stat.h>
#include <direct.h>
#include <functional>
#include <fstream>
#include "Text.h"
namespace File {
	typedef std::string FileStream;
	//创建文件
	extern bool Create(const Text::Utf8String& filename);
	//删除文件
	extern bool Delete(const Text::Utf8String& filename);
	//判断文件是否存在
	extern bool Exists(const Text::Utf8String& filename);
	//文件移动或者改名
	extern bool Move(const Text::Utf8String& oldname, const Text::Utf8String& newname);
	//读取文件并out返回
	extern void ReadFile(const  Text::Utf8String& filename, FileStream* fileStream);
	//写入文件
	extern void WriteFile(const FileStream* fileStream, const Text::Utf8String& filename);
	//拷贝文件
	extern void Copy(const  Text::Utf8String& filename, const  Text::Utf8String& des_filename);
}
namespace Path {
	//自己写的文件监控类
	class FileWatcher {
	private:
		Text::Utf8String math = "*.*";
		Text::Utf8String path;
		std::function<void(const Text::Utf8String& filename)> callback = NULL;
		size_t sleep;
		bool ctn = true;
		void TaskFunc();
	public:
		FileWatcher(const Text::Utf8String& path, const Text::Utf8String& math, const std::function<void(const Text::Utf8String& filename)>& callback, size_t sleep = 500);
		~FileWatcher();
	};
	//创建路径  MultiDir:是否创建多级目录
	extern bool Create(const Text::Utf8String& path);
	//删除路径 如果存在子文件夹或者文件 将会递归删除
	extern bool Delete(const Text::Utf8String& directoryName);
	//通配符搜索文件
	extern std::vector<Text::Utf8String> SearchFiles(const Text::Utf8String& path, const Text::Utf8String& pattern);
	//检查路径是否存在
	extern bool Exists(const Text::Utf8String& path);
	//获取文件名称(文件名称)
	extern Text::Utf8String GetFileNameWithoutExtension(const Text::Utf8String& _filename);
	//获取文件目录名称(所在目录)
	extern Text::Utf8String GetDirectoryName(const Text::Utf8String& _filename);
	//获取文件名称+后缀
	extern Text::Utf8String GetFileName(const Text::Utf8String& _filename);
	//获取文件后缀名(后缀名)
	extern Text::Utf8String GetExtension(const Text::Utf8String& _filename);
	//获取进程所在绝对路径目录
	extern Text::Utf8String StartPath();
	//获取进程所在绝对路径包含文件名称
	extern Text::Utf8String StartFileName();
#undef GetTempPath
	/// <summary>
	/// 获取应用程序的临时目录
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetTempPath();
	/// <summary>
	/// 获取应用程序数据存储目录 C:/Users/%s/AppData/Local/%s
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetAppDataPath();
};
namespace FileSystem {
	typedef enum : char {
		None,
		File,
		Directory
	}FileType;
	struct FileInfo
	{
	private:
		std::ifstream* fs = NULL;
	public:
		unsigned long long StreamPos = 0;
		struct _stat64 __stat;
		FileType FileType = FileSystem::FileType::None;
		Text::Utf8String Extension;
		Text::Utf8String FullName;
		Text::Utf8String FileName;
		bool ReadOnly = false;
		size_t Read(char* _buf_, size_t _rdCount = 512) {
			size_t rdbufCount = _rdCount;
			if (StreamPos + _rdCount >= __stat.st_size) {
				rdbufCount = __stat.st_size - StreamPos;
			}
			if (rdbufCount == 0) {
				return 0;
			}
			if (fs == NULL) {
				fs = new std::ifstream(FullName.utf16(), std::ios::binary);
			}
			fs->seekg(StreamPos);
			fs->read(_buf_, rdbufCount);
			StreamPos += rdbufCount;
			return rdbufCount;
		}
		FileInfo() {}
		FileInfo(const Text::Utf8String& filename) {
			int status = _wstat64(filename.utf16().c_str(), &__stat);
			if (status == 0 && (__stat.st_mode & S_IFREG) == S_IFREG) {
				Extension = Path::GetExtension(filename);
				FileName = Path::GetFileName(filename);
				FullName = filename;
				FileType = FileType::File;
			}
		}
		void Close() {
			if (fs) {
				fs->close();
				delete fs;
				fs = NULL;
			}
		}
		~FileInfo() {
			if (fs) {
				fs->close();
				delete fs;
			}
		}
	};
	extern void ReadFileInfoWin32(const Text::Utf8String& directory, WIN32_FIND_DATAW& pNextInfo, std::vector<FileSystem::FileInfo>& result);
	extern size_t  Find(const Text::Utf8String& directory, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern = "*.*");
}