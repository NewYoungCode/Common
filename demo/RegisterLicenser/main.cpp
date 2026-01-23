#include <sstream>
#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include "WinTool.h"
#include "JsonValue.h"
#include "Util.h"
#include "DateTime.h"

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	//for (size_t i = 0; i < argc; i++)
	//{
	//	::MessageBoxW(0, argv[i], L"", 0);
	//}
	Text::String fileName = argv[1];
	bool ok = WinTool::RegisterLicenser(fileName, std::to_string(::time(0)));
	if (ok) {
		::MessageBoxW(0, L"注册成功!", L"", 0);
	}
	return 0;
}