#pragma once
#include <malloc.h>
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <fstream> 
#include <io.h>
#include <tchar.h>
#include <vector>
#include <sstream>
#include "Text.h"
#include "FileSystem.h"
#pragma warning(disable:4996)
//操作ini文件的类
class ConfigIni {
private:
	size_t buffSize = 129;
	std::string filename;
	DWORD GetValue(const std::string & section, const std::string & key, const std::string & defaultValue, const std::string & filename, std::string & outResult) const;
	bool SetValue(const std::string & section, const std::string & key, const std::string & Value, const std::string & absoluteFilename) const;
public:
	void LoadFromFile(const std::string & filename, size_t buffSize);
	ConfigIni(const std::string & filename, size_t buffSize);
	ConfigIni();
	std::string ReadString(const std::string & key, const std::string & defaultValue, const std::string & section) const;
	float ReadFloat(const std::string & key, float defaultValue, const std::string & section) const;
	int ReadInt(const std::string & key, int defaultValue, const std::string & section) const;
	bool WriteValue(const std::string & key, const std::string & value, const std::string & section) const;
};
//常用工具
namespace Common {
	//设置开机启动
	bool SetAutoBoot(bool autoBoot = false);
	//获取是否自动启动状态
	bool GetAutoBootStatus();
	template<typename T >
	//申请二维数组
	T** MallocTwoArr(size_t row, size_t col);
	template<typename T >
	//是否二维数组长度
	void FreeTwoArr(T**arr, size_t row, size_t col);
	//打印日志
	void Log(const std::string &log);
}
//系统时间的工具
namespace DateTime {
	namespace Now {
		std::string ToString(const std::string &format = "yyyy-MM-dd hh::mm:ss");
	}
}
