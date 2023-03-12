#pragma once
#include "General.h"
#ifdef _DEBUG
#pragma comment(lib,"CommonD.lib")
#else
#pragma comment(lib,"Common.lib")
#endif

namespace comm {
	inline void LogA(const std::string &text) {
		OutputDebugStringA(text.c_str());
		OutputDebugStringA(("\n"));
		std::string appFilename = Path::StartFileName();
		std::string appName = Path::GetFileNameWithoutExtension(appFilename);

#ifndef _DEBUG
		//程序放在当前目录
		std::string dir = Path::GetDirectoryName(appFilename) + "\\" + appName + "_Log";
#else
		//程序放在Temp目录
		CHAR buf[MAX_PATH]{ 0 };
		::GetTempPathA(MAX_PATH, buf);
		std::string dir = std::string(buf) + "\\" + appName + "_Log";
#endif

		std::string logFilename = dir + "/" + Time::Now::ToString("yyyy-MM-dd") + ".log";
		Path::Create(dir);
		std::string logText = Time::Now::ToString("hh:mm:ss ") + text;
		std::ofstream file(logFilename.c_str(), std::ios::app);
		printf("%s\n", logText.c_str());
		file << logText.c_str() << std::endl;
		file.flush();
		file.close();
		//删除多余的日志 只保留当天的日志
		auto today = Time::Now::ToString("yyyy-MM-dd");
		auto files = Path::SearchFiles(dir, "*.log");
		for (auto&it : files) {
			auto t = Path::GetFileNameWithoutExtension(it);
			if (t.size() < 10 || t.substr(0, 10) != today) {
				File::Delete(it);
			}
		}
	}
	inline void LogW(const std::wstring &wtext) {
		LogA(Text::UnicodeToANSI(wtext));
	}

	template<typename ...T>
	inline void Log(const std::wstring& formatStr, T...args) {
		wchar_t buf[256]{ 0 };
		swprintf_s((buf), 255, formatStr.c_str(), std::forward<T>(args)...);
		LogW(buf);
	}

	template<typename ...T>
	inline void Log(const std::string& formatStr, T...args) {
		int len = formatStr.length();
		char *buf=new char[1024*1024*5]{ 0 };//5M的内存
		sprintf_s((buf), 1024 * 1024 * 5-1, formatStr.c_str(), std::forward<T>(args)...);
		LogA(buf);
		delete buf;
	}

}