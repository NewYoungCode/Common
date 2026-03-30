#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <fstream>
#include <vector>
#include <sstream>
#include <mutex>
#include <memory>
#include <map>
#include <io.h>
#include <Windows.h>
#include <winuser.h>
#include <assert.h>

#include "Text.h"
#include "FileSystem.h"
#include "DateTime.h"
#include "WinTool.h"
#include "WebClient.h"
#include "JsonCpp.hpp"
#include "Log.h"
#include "ThreadPool.hpp"
#include "IniConfig.h"

////如需使用fmt
//#include "fmt/format.h"
//#ifdef  _WIN64
//#ifdef  _DEBUG
//#pragma comment (lib,"X64/fmtd.lib")
//#else
//#pragma comment (lib,"X64/fmt.lib")
//#endif
//#else
//#ifdef _DEBUG
//#pragma comment(lib,"fmtd.lib")
//#else
//#pragma comment(lib,"fmt.lib")
//#endif // !_DEBUG
//#endif