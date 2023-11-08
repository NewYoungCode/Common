#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <iomanip>
#include <corecrt_io.h>
namespace MD5 {
	//MD5±àÂë
	std::string FromFile(const std::wstring& filename);
	std::string FromString(const std::string& string);
};