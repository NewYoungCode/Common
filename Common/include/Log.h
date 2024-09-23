#pragma once
#include "FileSystem.h"
#include "Time.hpp"
namespace Log {
	//是否启用日志
	extern bool Enable;
	extern void WriteLog(const Text::String& log);

	template<typename ...T>
	/// <summary>
	/// 打印utf8的字符
	/// </summary>
	/// <typeparam name="...T"></typeparam>
	/// <param name="formatStr"></param>
	/// <param name="...args"></param>
	inline void Info(const Text::String& formatStr, const T &...args) {
		if (!Enable)return;
		int size = 1024 * 1024 * 5;//5M的内存
		char* buf = new char[size] { 0 };
		auto count = sprintf_s((buf), size, formatStr.c_str(), std::forward<const T&>(args)...);
		buf[count] = '\n';
		buf[count + 1] = 0;
		Text::String info(buf);
		info = Time::Now().ToString("HH:mm:ss ") + info;
		delete[] buf;
		auto wstr = info.unicode();
		std::cout << info;
		OutputDebugStringW(wstr.c_str());
		WriteLog(info);
	}
};
