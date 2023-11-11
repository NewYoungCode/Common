#pragma once
#include <sys/stat.h>
#include <direct.h>
#include <functional>
#include <fstream>
#include "Text.h"
#include "Md5.h"
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
	//写入文件
	extern void WriteFile(const char* fileStream, size_t count, const Text::Utf8String& filename);
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
	//创建路径
	extern bool Create(const Text::Utf8String& path);
	//拷贝目录所有文件到目标目录
	extern bool Copy(const Text::Utf8String& srcPath, const Text::Utf8String& desPath);
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
	extern const Text::Utf8String& StartFileName();
#undef GetTempPath
	/// <summary>
	/// 获取应当前windows用户的临时目录
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetTempPath();
	/// <summary>
	/// 获取应用程序的临时目录
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetAppTempPath();
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
	public:
		FileType FileType = FileSystem::FileType::None;
		Text::Utf8String Extension;
		Text::Utf8String FullName;
		Text::Utf8String FileName;
		ULONGLONG FileSize = 0;
		bool ReadOnly = false;
		FileInfo(const Text::Utf8String& fileName) {
			if (File::Exists(fileName)) {
				Extension = Path::GetExtension(fileName);
				FileName = Path::GetFileName(fileName);
				FullName = fileName;
				FileType = FileType::File;
				std::ifstream ifs(fileName.unicode(), std::ios::binary);
				ifs.seekg(0, std::ios::end);
				FileSize = ifs.tellg();
			}
		}
		FileInfo() {}
		void Close() {
		}
		~FileInfo() {
		}
	};
	extern size_t Find(const Text::Utf8String& directory, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern = "*.*", bool loopSubDir = false);
}