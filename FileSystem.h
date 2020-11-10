#pragma once
#include <malloc.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <fstream>
#include <io.h>
#include <tchar.h>
#include <vector>
#include <sstream>
#include "Text.h"
namespace FileSystem {
	typedef enum :short {
		None,
		File,
		Directory
	}FileType;
	struct FileInfo
	{
		FileType FileType = FileSystem::FileType::None;
		std::string Extension;
		std::string FullName;
		std::string FileName;
		bool ReadOnly = false;
	};
	void ReadFileInfo(const std::string& directory, WIN32_FIND_DATAA&pNextInfo, std::vector<FileSystem::FileInfo>&result);
	size_t  Find(const std::string& directory, std::vector<FileSystem::FileInfo>&result, const char* pattern = "*.*");
}
namespace File {
	//创建文件
	bool Create(const std::string &filename);
	//读取文件
	std::string ReadFile(const char* filename);
	//删除文件
	bool Delete(const std::string &filename);
	//判断文件是否存在
	bool Exists(const std::string&filename);
	//文件移动或者改名
	bool Move(const std::string &oldname, const std::string &newname);
	//读取文件并out返回
	void ReadFile(const  std::string &filename, std::string&outData);
	//写入文件
	void WriteFile(const std::stringstream & data, const std::string & filename);
}
namespace Path {
	//创建路径  MultiDir:是否创建多级目录
	bool Create(const std::string &path, bool MultiDir = false);
	//删除路径 如果存在子文件夹或者文件 将会递归删除
	bool Delete(const std::string& directoryName);
	//通配符搜索文件
	std::vector<std::string> SearchFiles(const std::string &path, const char*pattern);
	//检查路径是否存在
	bool Exists(const std::string &path);
	//获取文件名称(文件名称)
	std::string GetFileNameWithoutExtension(const std::string &_filename);
	//获取文件目录名称(所在目录)
	std::string GetDirectoryName(const std::string &_filename);
	//获取文件名称+后缀
	std::string GetFileName(const std::string &_filename);
	//获取文件后缀名(后缀名)
	std::string GetExtension(const std::string &_filename);
	//获取进程所在绝对路径目录
	std::string	StartPath();
	//获取进程所在绝对路径Fullpath
	std::string	GetModuleFileName();
}
