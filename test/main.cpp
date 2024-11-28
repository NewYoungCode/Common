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

	//�����ַ���
std::string inputString(const std::string tips) {
	std::string input;
	std::cout << tips;
	std::cin >> input;
	return input;
}

// ��ȡռ���ļ��Ľ���PID
std::vector<DWORD> getPid(const Text::String& filePath) {
	std::vector<DWORD> pidList;
	Text::String str = WinTool::ExecuteCmdLine("handle.exe " + filePath);
	auto lines = str.split("\n");
	for (auto& it : lines) {
		auto line = it;
		auto pos = line.find("pid: ");
		if (pos != -1) {
			line = line.substr(pos + 5);
			pos = line.find(" ");
			line = line.substr(0, pos);
			pidList.push_back(std::atoi(line.c_str()));
		}
	}
	return pidList;
}

bool IsDirectoryRedirected(const std::wstring& directory, Text::String& out) {

	HANDLE hFile = CreateFileW(directory.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		auto code = ::GetLastError();
		Log::Info("code %d �޷���·��  %s", code, Text::String(directory).c_str());
		return true; // �����ʧ�ܣ����� false
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
bool redirect(const Text::String& linkName, const Text::String& target, char disk) {

	//��ȡ�û�����
	WCHAR user[256]{ 0 };
	DWORD len = 256;
	::GetUserNameW(user, &len);
	std::wstring userName = user;

	//�滻�û������̷�
	Text::Replace((Text::String*)&linkName, "{user}", Text::String(user));
	Text::Replace((Text::String*)&target, "{user}", Text::String(user));
	((Text::String&)target)[0] = disk;

	//����ռ�õĽ���id
	std::vector<DWORD> pids = getPid(linkName);
	for (auto& pid : pids) {
		Log::Info("ɱ������ %d", pid);
		WinTool::CloseProcess(pid);
	}

	//�ж��ǲ����Ѿ��ض������
	Text::String out;
	if (IsDirectoryRedirected(linkName.unicode(), out)) {
		if (Path::Equal(target, out)) {
			return true;
		}
		else {
			Log::Info("[���е��ض�����ȷ,ִ��ɾ��!]%s->%s", linkName.c_str(), out.c_str());
			Path::Delete(linkName);
		}
	}

	auto ok = Path::Copy(linkName, target);//�ȿ���
	ok = Path::Delete(linkName);//ɾ��

	Text::String cmd = "cmd.exe /c mklink /j " + linkName + " " + target;
	auto ret = WinTool::ExecuteCmdLine(cmd);

	Log::Info("%s", ret.c_str());
	return ret.empty();
}

//�رմ���Ϸ��ʱ�򵯳���GameBar
void closeGameBar() {

	{
		//�����\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\GameDVR  (AppCaptureEnabled 32dword���� ��Ϊ0)
		HKEY subKey = NULL;
		do
		{
			if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR", NULL, KEY_ALL_ACCESS, &subKey)) {
				break;
			}
			//ɾ��AppCaptureEnabled
			::RegDeleteValueW(subKey, L"AppCaptureEnabled");
			DWORD value = 0; // Ҫ���õ�ֵ
			if (::RegSetValueExW(subKey, L"AppCaptureEnabled", 0, REG_DWORD, (const BYTE*)&value, sizeof(value))) {
				break;
			}
		} while (false);
		::RegCloseKey(subKey);
	}

	{
		//�����\HKEY_CURRENT_USER\System\GameConfigStore  (GameDVR_Enabled ��Ϊ0)
		HKEY subKey = NULL;
		do
		{
			if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CURRENT_USER, L"System\\GameConfigStore", NULL, KEY_ALL_ACCESS, &subKey)) {
				break;
			}
			//ɾ��GameDVR_Enabled
			::RegDeleteValueW(subKey, L"GameDVR_Enabled");
			DWORD value = 0; // Ҫ���õ�ֵ
			if (::RegSetValueExW(subKey, L"GameDVR_Enabled", 0, REG_DWORD, (const BYTE*)&value, sizeof(value))) {
				break;
			}
		} while (false);
		::RegCloseKey(subKey);
	}

	{
		//�ر������"�˽��ͼƬ"
		//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\HideDesktopIcons\NewStartPanel\{2cc5ca98-6485-489a-920e-b3e88a6ccce3} DWORD32 1 �ر�
		HKEY subKey = NULL;
		do
		{
			if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel", NULL, KEY_ALL_ACCESS, &subKey)) {
				break;
			}
			::RegDeleteValueW(subKey, L"{2cc5ca98-6485-489a-920e-b3e88a6ccce3}");
			DWORD value = 1; // Ҫ���õ�ֵ
			if (::RegSetValueExW(subKey, L"{2cc5ca98-6485-489a-920e-b3e88a6ccce3}", 0, REG_DWORD, (const BYTE*)&value, sizeof(value))) {
				break;
			}
		} while (false);
		::RegCloseKey(subKey);
	}
	{
		//������ʾ�ҵ�"�ҵĵ���"
		//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\HideDesktopIcons\NewStartPanel\{20D04FE0-3AEA-1069-A2D8-08002B30309D} DWORD32 0��ʾ�ҵĵ���
		HKEY subKey = NULL;
		do
		{
			if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel", NULL, KEY_ALL_ACCESS, &subKey)) {
				break;
			}
			::RegDeleteValueW(subKey, L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}");
			DWORD value = 0; // Ҫ���õ�ֵ
			if (::RegSetValueExW(subKey, L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}", 0, REG_DWORD, (const BYTE*)&value, sizeof(value))) {
				break;
			}
		} while (false);
		::RegCloseKey(subKey);
	}
}

int main() {

	Log::Enable = true;

	system("title ϵͳ�Ż�����");

	auto a = inputString("C�������ƶ����ĸ��̷�?(A-Z):");

	char disk = 'D';//ת�Ƶ����̷�
	if (!a.empty()) {
		disk = a[0];
	}

	if (disk == 'C' || disk == 'c') {
		Log::Info("�������������?");
		system("pause");
		return 0;
	}

	Log::Info("�����C�̲��������ƶ���:[%c]��(����Ӱ��ԭ������)", disk);
	system("pause");

	//����Ϸ��ʱ��ر�gamebar�ĵ���
	closeGameBar();

	//�ر�ϵͳ����
	system("cmd.exe /c powercfg -h off");

	redirect(L"C:\\Users\\{user}\\Documents", L"?:\\Users\\{user}\\Documents", disk);//�ĵ�Ŀ¼
	redirect(L"C:\\Users\\{user}\\Downloads", L"?:\\Users\\{user}\\Downloads", disk);//����Ŀ¼
	redirect(L"C:\\Users\\{user}\\Music", L"?:\\Users\\{user}\\Music", disk);//��ƵĿ¼
	redirect(L"C:\\Users\\{user}\\Videos", L"?:\\Users\\{user}\\Videos", disk);//��ƵĿ¼
	redirect(L"C:\\Users\\{user}\\Pictures", L"?:\\Users\\{user}\\Pictures", disk);//��ƬĿ¼
	redirect(L"C:\\Users\\{user}\\AppData\\Local\\Temp", L"?:\\Users\\{user}\\AppData\\Local\\Temp", disk);//TempĿ¼
	//system("explorer.exe");
	system("pause");
	return 0;
}