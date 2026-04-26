#pragma once
#include <iostream>
#include "FileSystem.h"
#include "DateTime.h"
namespace Log {
	//是否将输出写入日志文件
	extern bool WriteFile;
	extern void WriteLog(const Text::String& log);

	/// <summary>
	/// 打印utf8的字符
	/// </summary>
	/// <typeparam name="...T"></typeparam>
	/// <param name="formatStr"></param>
	/// <param name="...args"></param>
	template<typename ...T>
	inline Text::String Info(const Text::String& formatStr, const T &...args) {
		Text::String info;
		size_t count = sizeof...(args);
		if (count > 0) {
			// 计算格式化后的字符串所需的内存大小
			int bufSize = ::snprintf(nullptr, 0, formatStr.c_str(), args...) + 2;  // +1是为了换行符和结束符 \n '\0'
			char* buf = new char[bufSize];
			auto count = ::sprintf_s(buf, bufSize, formatStr.c_str(), std::forward<const T&>(args)...);
			buf[count] = '\n';
			buf[count + 1] = 0;
			info = buf;
			delete[] buf;
		}
		else {
			info = formatStr;
		}
		info = DateTime::Now().ToString("HH:mm:ss ") + info;
		//转为本地可识别的编码
		auto ansi = info.ansi();
		std::cout << ansi;
		OutputDebugStringA(ansi.c_str());
		WriteLog(info);
		return info;
	}

	/// <summary>
	/// 打印utf8的字符(仅Debug模式下生效)
	/// </summary>
	/// <typeparam name="...T"></typeparam>
	/// <param name="formatStr"></param>
	/// <param name="...args"></param>
	template<typename ...T>
	inline void Debug(const Text::String& formatStr, const T &...args) {
#ifdef  _DEBUG
		Text::String info;
		size_t count = sizeof...(args);
		if (count > 0) {
			// 计算格式化后的字符串所需的内存大小
			int bufSize = ::snprintf(nullptr, 0, formatStr.c_str(), args...) + 2;  // +1是为了换行符和结束符 \n '\0'
			char* buf = new char[bufSize];
			auto count = ::sprintf_s(buf, bufSize, formatStr.c_str(), std::forward<const T&>(args)...);
			buf[count] = '\n';
			buf[count + 1] = 0;
			info = buf;
			delete[] buf;
		}
		else {
			info = formatStr;
		}
		info = DateTime::Now().ToString("HH:mm:ss ") + info;
		//转为本地可识别的编码
		auto ansi = info.ansi();
		std::cout << ansi;
		OutputDebugStringA(ansi.c_str());
#endif //  _DEBUG
	}
};
