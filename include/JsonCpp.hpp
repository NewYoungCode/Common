#pragma once
#include <string>
#include <Windows.h>

#define USEJSONCPP 1 //是否使用JsonCpp库 

#if USEJSONCPP
#include <json/json.h>

#ifdef  _WIN64

#ifdef  _DEBUG
#pragma comment (lib,"X64/jsoncpp/Debug/jsoncpp.lib")
#else
#pragma comment (lib,"X64/jsoncpp/Release/jsoncpp.lib")
#endif

#else

#ifdef  _DEBUG
#pragma comment (lib,"jsoncpp/Debug/jsoncpp.lib")
#else
#pragma comment (lib,"jsoncpp/Release/jsoncpp.lib")
#endif

#endif

namespace Json {
	//字符串转换成json对象 失败则返回Json::nullValue
	inline Json::Value Parse(const std::string& jsonStr)
	{
		Json::Reader rd;
		Json::Value out;
		if (rd.parse(jsonStr, out)) {
			return out;
		}
		return Json::nullValue;
	}
};
#endif