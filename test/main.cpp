//#include <sstream>
//
//#include "WebClient.h"
//#include "FileSystem.h"
//#include "Text.h"
//#include "WinTool.h"
//#include "JsonValue.h"
//#include "QrenCode.hpp"
//
//int main() {
//
//	Text::String linkName = L"C:\\Users\\ly\\Downloads";
//	Text::String target = L"D:\\down";
//
//	Text::String cmd = "cmd.exe /c mklink /j " + linkName + " " + target;
//	auto ret = WinTool::ExecuteCmdLine(cmd);
//
//
//	//����openssl,zlib��
//
//	std::cout << "PCID:" << WinTool::GetComputerID() << std::endl;
//
//	//��ά���ʹ��
//	//QrenCode::Generate("https://www.baidu.com", L"d:/aa.bmp");
//	auto bmp = QrenCode::Generate("https://www.baidu.com");
//	::DeleteObject(bmp);
//
//	WinTool::GetWinVersion();
//
//	JsonValue obj("aa");
//	Json::Value jv;
//
//	WebClient wc;
//
//	Text::String resp;
//	wc.HttpGet("https://songsearch.kugou.com/song_search_v2?platform=WebFilter&pagesize=20&page=1&keyword=dj", &resp);
//
//	auto w = resp.ansi();
//	std::cout << w;
//
//	obj = jv;
//
//	return 0;
//}

#include <sstream>
#include "Text.h"
#include "WinTool.h"
#include "Log.h"

bool IsDirectoryRedirected(const std::wstring& directory, Text::String& out) {
	HANDLE hFile = CreateFileW(directory.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		auto code = ::GetLastError();
		if (code == 32) {

		}
		return false; // �����ʧ�ܣ����� false
	}
	WCHAR finalPath[MAX_PATH];
	DWORD pathLength = GetFinalPathNameByHandleW(hFile, finalPath, MAX_PATH, FILE_NAME_NORMALIZED);
	CloseHandle(hFile);
	if (pathLength == 0) {
		return false;
	}
	std::wstring path = finalPath;
	size_t pos = path.find(L"\\\\?\\");
	if (pos != -1) {
		path = path.substr(4);
	}
	out = path;
	return  Text::String(directory) != out; // ���·����ͬ��˵����Ŀ¼���ض���
}
bool redirect(const Text::String& linkName, const Text::String& target) {

	WCHAR user[256]{ 0 };
	DWORD len = 256;
	::GetUserNameW(user, &len);
	std::wstring userName = user;

	Text::Replace((Text::String*)&linkName, "{user}", Text::String(user));
	Text::Replace((Text::String*)&target, "{user}", Text::String(user));

	Text::String out;
	if (IsDirectoryRedirected(linkName.unicode(), out)) {
		if (Path::Equal(target, out)) {
			return true;
		}
		Log::Info("��Ŀ¼�Ѿ��ض���!%s->%s", linkName.c_str(), out.c_str());
		Path::Delete(linkName);
	}

	Path::Create(target);
	auto ok = Path::Copy(linkName, target);//�ȿ���
	ok = Path::Delete(linkName);//ɾ��
	Text::String cmd = "cmd.exe /c mklink /j " + linkName + " " + target;
	auto ret = WinTool::ExecuteCmdLine(cmd);

	Log::Info("%s", ret.c_str());
	return ret.empty();
}

int main() {

	//system("taskkill /f /pid explorer.exe");
	Log::Enable = true;
	WCHAR user[256]{ 0 };
	DWORD len = 256;
	::GetUserNameW(user, &len);
	std::wstring userName = user;

	Text::String userPath = L"D:\\Users\\{user}";
	Text::Replace((Text::String*)&userPath, "{user}", Text::String(user));
	Path::Create(userPath);

	redirect(L"C:\\Users\\{user}\\Documents", L"D:\\Users\\{user}\\Documents");//�ĵ�Ŀ¼
	redirect(L"C:\\Users\\{user}\\Downloads", L"D:\\Users\\{user}\\Downloads");//����Ŀ¼
	redirect(L"C:\\Users\\{user}\\Music", L"D:\\Users\\{user}\\Music");//��ƵĿ¼
	redirect(L"C:\\Users\\{user}\\Videos", L"D:\\Users\\{user}\\Videos");//��ƵĿ¼
	redirect(L"C:\\Users\\{user}\\Pictures", L"D:\\Users\\{user}\\Pictures");//��ƬĿ¼
	redirect(L"C:\\Users\\{user}\\Desktop", L"D:\\Users\\{user}\\Desktop");//����Ŀ¼
	redirect(L"C:\\Users\\{user}\\AppData\\Local\\Temp", L"D:\\Users\\{user}\\AppData\\Local\\Temp");//TempĿ¼
	//system("explorer.exe");
	system("pause");
	return 0;
}