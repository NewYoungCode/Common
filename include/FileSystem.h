#pragma once
#include <sys/stat.h>
#include <direct.h>
#include <functional>
#include <fstream>
#include "Text.h"
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
	extern bool Copy(const  Text::String& filename, const  Text::String& des_filename, bool overwrite = true);
}
namespace Path {
	//创建路径
	extern bool Create(const Text::String& path);
	//拷贝目录所有文件到目标目录
	extern bool Copy(const Text::String& srcPath, const Text::String& desPath, bool overwrite = true);
	//移动目录到新位置
	extern bool Move(const Text::String& oldname, const Text::String& newname);
	//删除路径 如果存在子文件夹或者文件 将会递归删除
	extern bool Delete(const Text::String& directoryName);
	//通配符搜索文件
	extern std::vector<Text::String> SearchFiles(const Text::String& path, const Text::String& pattern);
	//检查路径是否存在
	extern bool Exists(const Text::String& path);
	//判断路径是不是相同
	extern bool Equal(const Text::String& path1, const Text::String& path2);
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
	typedef enum {
		None,
		File,
		Directory
	}FileType;
	class FileInfo
	{
	private:
		std::ifstream* ifs = NULL;
		ULONGLONG StreamPos = 0;
	public:
		const FileType FileType = FileSystem::FileType::None;
		const Text::String Extension;
		const Text::String FullName;
		const Text::String FileName;
		const bool ReadOnly = false;

		const ULONGLONG FileSize = 0;
		FileInfo() {}
		FileInfo(const Text::String& fileName) {
			if (File::Exists(fileName)) {
				(Text::String)Extension = Path::GetExtension(fileName);
				(Text::String)FileName = Path::GetFileName(fileName);
				(Text::String)FullName = fileName;
				(FileSystem::FileType)FileType = FileType::File;
				ifs = new std::ifstream(fileName.unicode(), std::ios::binary);
				ifs->seekg(0, std::ios::end);
				(ULONGLONG)FileSize = ifs->tellg();
			}
		}
		size_t Read(char* _buf_, size_t _rdCount = 256) {
			size_t rdbufCount = _rdCount;
			if (StreamPos + _rdCount >= FileSize) {
				rdbufCount = FileSize - StreamPos;
			}
			if (rdbufCount == 0) {
				return 0;
			}
			if (ifs == NULL) {
				ifs = new std::ifstream(FullName.unicode(), std::ios::binary);
			}
			ifs->seekg(StreamPos);
			ifs->read(_buf_, rdbufCount);
			StreamPos += rdbufCount;
			return rdbufCount;
		}
		void Close() {
			ifs->close();
		}
		~FileInfo() {
			if (ifs) {
				delete ifs;
			}
		}
	};
	extern size_t Find(const Text::String& directory, std::vector<FileSystem::FileInfo>& result, const Text::String& pattern = "*.*", bool loopSubDir = false);
}