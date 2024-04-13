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
	extern bool Create(const Text::String& filename);
	//删除文件
	extern bool Delete(const Text::String& filename);
	//判断文件是否存在
	extern bool Exists(const Text::String& filename);
	//文件移动或者改名
	extern bool Move(const Text::String& oldname, const Text::String& newname);
	//读取文件并out返回
	extern void ReadFile(const  Text::String& filename, FileStream* fileStream);
	//写入文件
	extern void WriteFile(const FileStream* fileStream, const Text::String& filename);
	//写入文件
	extern void WriteFile(const char* fileStream, size_t count, const Text::String& filename);
	//拷贝文件
	extern void Copy(const  Text::String& filename, const  Text::String& des_filename);
}
namespace Path {
	//创建路径
	extern bool Create(const Text::String& path);
	//拷贝目录所有文件到目标目录
	extern bool Copy(const Text::String& srcPath, const Text::String& desPath);
	//移动目录到新位置
	extern bool Move(const Text::String& oldname, const Text::String& newname);
	//删除路径 如果存在子文件夹或者文件 将会递归删除
	extern bool Delete(const Text::String& directoryName);
	//通配符搜索文件
	extern std::vector<Text::String> SearchFiles(const Text::String& path, const Text::String& pattern);
	//检查路径是否存在
	extern bool Exists(const Text::String& path);
	//获取文件名称(文件名称)
	extern Text::String GetFileNameWithoutExtension(const Text::String& _filename);
	//获取文件目录名称(所在目录)
	extern Text::String GetDirectoryName(const Text::String& _filename);
	//获取文件名称+后缀
	extern Text::String GetFileName(const Text::String& _filename);
	//获取文件后缀名(后缀名)
	extern Text::String GetExtension(const Text::String& _filename);
	//获取进程所在绝对路径目录
	extern Text::String StartPath();
	//获取进程所在绝对路径包含文件名称
	extern const Text::String& StartFileName();
#undef GetTempPath
	/// <summary>
	/// 获取应当前windows用户的临时目录
	/// </summary>
	/// <returns></returns>
	extern Text::String GetTempPath();
	/// <summary>
	/// 获取应用程序的临时目录
	/// </summary>
	/// <returns></returns>
	extern Text::String GetAppTempPath();
	/// <summary>
	/// 获取应用程序数据存储目录 C:/Users/%s/AppData/Local/%s
	/// </summary>
	/// <returns></returns>
	extern Text::String GetAppDataPath();
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
		Text::String Extension;
		Text::String FullName;
		Text::String FileName;
		ULONGLONG FileSize = 0;
		bool ReadOnly = false;
		FileInfo(const Text::String& fileName) {
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
	extern size_t Find(const Text::String& directory, std::vector<FileSystem::FileInfo>& result, const Text::String& pattern = "*.*", bool loopSubDir = false);
}