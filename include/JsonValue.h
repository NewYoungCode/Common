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


struct JsonValue :public Json::Value {
private:
	bool b = false;
	Json::Reader rd;
public:
	bool IsJson();
	//JObject(const Json::Value& right);
	JsonValue& operator = (const Json::Value& other);
	JsonValue(const std::string& jsonStr);
};
#endif