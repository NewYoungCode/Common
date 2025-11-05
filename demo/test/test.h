#pragma once
#include <windows.h>
#include <iostream>

#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include "WinTool.h"
#include "JsonValue.h"
#include "QrenCode.hpp"
#include "Log.h"

#include <iostream>
#include <string>
#include <search.h>
#include <algorithm>

//读取命令行所需
#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#pragma comment(lib, "wbemuuid.lib")

namespace test {
	std::wstring findCommandLine(const std::wstring& exeName);
};