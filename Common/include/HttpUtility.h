#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <iomanip>
#include <corecrt_io.h>

namespace HttpUtility {
	// NOTE: This code came up with the following stackoverflow post:
	// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
	std::string base64_encode(const unsigned char* text, unsigned int text_len);
	std::string  base64_decode(const unsigned char* code, unsigned int code_len);
	//url����
	std::string UrlEncode(const std::string& str);
	//url����
	std::string UrlDecode(const std::string& str, bool convert_plus_to_space = false);
	//MD5����
	std::string Md5EncodeFormFile(const std::string& filename);
	std::string Md5Encode(const std::string& strdata);
	//��һ��GZIP
};