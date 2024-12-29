#include "Windows.h"
#include "Log.h"
#include "Text.h"
#include "WinTool.h"

bool isRunning(const std::string& pathA) {
	auto pids = WinTool::FindProcessId(Path::GetFileName(pathA));
	bool isRun = false;
	for (auto& it : pids) {
		auto pathB = WinTool::FindProcessFilename(it);
		if (pathA == pathB) {
			isRun = true;
			break;
		}
	}
	return isRun;
}

int main() {

	HWND hwnd = ::GetConsoleWindow();//获取控制台句柄

	Log::Enable = true;
	std::vector<std::string> list;

	std::string fileData;

	auto startFileName = Path::StartFileName();
	Text::String listFileName = Path::GetDirectoryName(startFileName) + "\\" + Path::GetFileNameWithoutExtension(startFileName) + ".list";
	if (!File::Exists(listFileName)) {
		::MessageBoxW(hwnd, listFileName.unicode().c_str(), L"file doesn't exist", MB_ICONSTOP);
		return 0;
	}

	File::ReadFile(listFileName, &fileData);

	//去除utf8-bom头
	char bom[]{ 0xef,0xbb, 0xbf,0 };
	auto pos = fileData.find(bom);
	if (pos != size_t(-1)) {
		fileData = fileData.substr(3);
	}

	Text::Replace(&fileData, "\r", "");
	Text::Split(fileData, "\n", &list);

	for (auto& it : list) {
		Log::Info(it);
		it = Path::StartPath() + "\\" + it;
	}
	Log::Info(L"守护程序运行中... 请勿关闭!");
	::ShowWindow(hwnd, SW_HIDE);
	while (true)
	{
		for (auto& it : list) {
			if (!isRunning(it)) {
				Text::String fileName = Path::GetFileName(it);
				Log::Info(L"[%s]程序已停止,重新启动!", fileName.c_str());
				auto result = ::ShellExecuteW(NULL, L"open", fileName.unicode().c_str(), L"", L"", SW_SHOW);
				// 如果返回值小于或等于32
				if ((int)result <= 32) {
					Log::Info(L"[%s]启动失败%d", fileName.c_str(), (int)result);
				}
				else {
					Log::Info(L"[%s]启动成功", fileName.c_str());
				}
			}
			Sleep(1000);
		}
	}
	return 0;
}

//
//int main() {
//	Log::Enable = true;
//	std::vector<std::string> list;
//
//	std::string fileData;
//	File::ReadFile(Path::StartPath() + "\\list.txt", &fileData);
//	Text::Split(fileData, "\n", &list);
//
//	for (auto& it : list) {
//		Log::Info(it);
//		it = Path::StartPath() + "\\" + it;
//	}
//	Log::Info(L"守护程序运行中... 请勿关闭!");
//	HWND hwnd = ::GetConsoleWindow();//获取控制台句柄
//	//::ShowWindow(hwnd, SW_HIDE);
//	while (true)
//	{
//		for (auto& it : list) {
//			if (!isRunning(it)) {
//				Text::String fileName = Path::GetFileName(it);
//				Log::Info(L"[%s]程序已停止,重新启动!", fileName.c_str());
//				auto result = ::ShellExecuteW(NULL, L"open", fileName.unicode().c_str(), L"", L"", SW_SHOW);
//				// 如果返回值小于或等于32
//				if ((int)result <= 32) {
//					Log::Info(L"[%s]启动失败%d", fileName.c_str(), (int)result);
//				}
//				else {
//					Log::Info(L"[%s]启动成功", fileName.c_str());
//				}
//			}
//			Sleep(2000);
//		}
//	}
//	return 0;
//}