#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <iomanip>
#include <corecrt_io.h>
namespace Util {
	//文件的md5
	std::string MD5FromFile(const std::wstring& filename);
	//字符串的md5
	std::string MD5FromString(const std::string& string);
	//base64编码
	std::string Base64Encode(const char* str, int strLen);
	//base64解码
	std::string  Base64Decode(const char* base64Str, int strLen);
	//url编码
	std::string UrlEncode(const std::string& str);
	//url解码
	std::string UrlDecode(const std::string& str, bool convert_plus_to_space = false);
	//少一个GZIP
};