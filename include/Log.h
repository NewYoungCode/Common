#pragma once
#include <iostream>
#include <mutex>
#include "FileSystem.h"
#include "DateTime.h"
#include "fmt.hpp"
namespace Log {
	//是否将输出写入日志文件
	extern bool WriteFile;
	extern void WriteLog(const Text::String& log);
	extern std::mutex global_log_mutex;
	/// <summary>
	/// 使用fmt库格式化且打印utf8的字符
	/// </summary>
	/// <typeparam name="...T"></typeparam>
	/// <param name="formatStr"></param>
	/// <param name="...args"></param>
	template<typename ...T>
	inline Text::String Info(const Text::String& formatStr, const T &...args) {
		std::lock_guard<std::mutex> autoLock(global_log_mutex);
		Text::String info;
		size_t count = sizeof...(args);
		if (count > 0) {
			info = fmt::format(formatStr, std::forward<const T&>(args)...) + "\n";
		}
		else {
			info = formatStr + "\n";
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
	/// 使用fmt库格式化且打印utf8的字符(仅Debug模式下生效)
	/// </summary>
	/// <typeparam name="...T"></typeparam>
	/// <param name="formatStr"></param>
	/// <param name="...args"></param>
	template<typename ...T>
	inline void Debug(const Text::String& formatStr, const T &...args) {
		std::lock_guard<std::mutex> autoLock(global_log_mutex);
#ifdef  _DEBUG
		Text::String info;
		size_t count = sizeof...(args);
		if (count > 0) {
			info = fmt::format(formatStr, std::forward<const T&>(args)...) + "\n";
		}
		else {
			info = formatStr + "\n";
		}
		info = DateTime::Now().ToString("HH:mm:ss ") + info;
		//转为本地可识别的编码
		auto ansi = info.ansi();
		std::cout << ansi;
		OutputDebugStringA(ansi.c_str());
#endif //  _DEBUG
	}
};
