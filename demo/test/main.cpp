#include "test.h"

//int main() {
//
//	Text::String linkName = L"C:\\Users\\ly\\Downloads";
//	Text::String target = L"D:\\down";
//
//	Text::String cmd = "cmd.exe /c mklink /j " + linkName + " " + target;
//	auto ret = WinTool::ExecuteCmdLine(cmd);
//
//	//新增openssl,zlib库
//
//	std::cout << "PCID:" << WinTool::GetComputerID() << std::endl;
//
//	//二维码的使用
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
//	//获取com端口名称
//	auto coms = WinTool::GetComPorts();
//
//	return 0;
//}
//
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#pragma comment(lib, "wbemuuid.lib")

std::wstring lastCmd;
IWbemServices* pSvc = nullptr;

bool initWMI() {
	HRESULT hres;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return false;

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, NULL);
	if (FAILED(hres)) return false;

	IWbemLocator* pLoc = nullptr;
	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) return false;

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL, NULL, 0, NULL, 0, 0, &pSvc);
	pLoc->Release();
	if (FAILED(hres)) return false;

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
		NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
	if (FAILED(hres)) {
		pSvc->Release();
		pSvc = nullptr;
		return false;
	}

	return true;
}

void find(const std::wstring& exeName) {
	if (!pSvc) return;

	IEnumWbemClassObject* pEnumerator = nullptr;
	HRESULT hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT ProcessId, Name, CommandLine FROM Win32_Process"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres)) return;

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;

	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
		VARIANT vtName;
		HRESULT hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
		if (SUCCEEDED(hr) && vtName.vt == VT_BSTR &&
			exeName == vtName.bstrVal) {

			VARIANT vtCmd;
			hr = pclsObj->Get(L"CommandLine", 0, &vtCmd, 0, 0);
			if (SUCCEEDED(hr) && vtCmd.vt == VT_BSTR) {
				if (lastCmd != vtCmd.bstrVal && ::lstrlenW(vtCmd.bstrVal) != 0) {
					std::wcout << L"CommandLine: " << vtCmd.bstrVal << std::endl;
					lastCmd = vtCmd.bstrVal;

					VARIANT vtPid;
					hr = pclsObj->Get(L"ProcessId", 0, &vtPid, 0, 0);
					if (SUCCEEDED(hr)) {
						std::wcout << L"PID: " << vtPid.uintVal << std::endl;
					}
					VariantClear(&vtPid);
					std::wcout << L"-----------------------------" << std::endl;
				}
			}
			VariantClear(&vtCmd);
		}
		VariantClear(&vtName);
		pclsObj->Release();
	}

	pEnumerator->Release();
}

#pragma comment(lib, "wbemuuid.lib")
// 获取 WMI 中的 UUID（设备标识符）
std::string GetDeviceUUID() {
	std::string result;
	HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return "";

	hres = CoInitializeSecurity(
		NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, NULL);

	if (FAILED(hres)) {
		CoUninitialize();
		return "";
	}

	IWbemLocator* pLoc = nullptr;
	hres = CoCreateInstance(CLSID_WbemLocator, 0,
		CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return "";
	}

	IWbemServices* pSvc = nullptr;
	hres = pLoc->ConnectServer(
		BSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return "";
	}

	hres = CoSetProxyBlanket(
		pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);

	IEnumWbemClassObject* pEnumerator = nullptr;
	hres = pSvc->ExecQuery(
		BSTR(L"WQL"),
		BSTR(L"SELECT UUID FROM Win32_ComputerSystemProduct"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL, &pEnumerator);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "";
	}

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;

	if (pEnumerator) {
		while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
			VARIANT vtProp;
			VariantInit(&vtProp);
			hres = pclsObj->Get(L"UUID", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hres) && vtProp.vt == VT_BSTR) {
				std::wstring wstr(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
				result = Text::String(wstr);
			}
			VariantClear(&vtProp);
			pclsObj->Release();
		}
		pEnumerator->Release();
	}

	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return result;
}
std::string GetMotherboardID() {
	HRESULT hres;

	// 初始化 COM
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return "COM init failed";

	// 设置默认安全性
	hres = CoInitializeSecurity(
		NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, NULL);
	if (FAILED(hres)) {
		CoUninitialize();
		return "Security init failed";
	}

	IWbemLocator* pLoc = nullptr;
	hres = CoCreateInstance(
		CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return "WbemLocator creation failed";
	}

	IWbemServices* pSvc = nullptr;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return "WMI connection failed";
	}

	hres = CoSetProxyBlanket(
		pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
		NULL, RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "SetProxyBlanket failed";
	}

	IEnumWbemClassObject* pEnumerator = nullptr;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT SerialNumber FROM Win32_BaseBoard"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL, &pEnumerator);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "Query failed";
	}

	IWbemClassObject* pClassObject = nullptr;
	ULONG uReturn = 0;
	std::string serialNumber = "unknown";

	if (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
		if (uReturn > 0 && SUCCEEDED(hr)) {
			VARIANT vtProp;
			hr = pClassObject->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
				serialNumber = _bstr_t(vtProp.bstrVal);
				VariantClear(&vtProp);
			}
			pClassObject->Release();
		}
		pEnumerator->Release();
	}

	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return serialNumber;
}
int main() {

	auto sssss = Path::StartPrograms();
	auto sssss2 = Path::UserDesktop();

	bool bind = WinTool::CheckDebug();

	WinTool::AddFirewallRule(Path::StartFileName());

	WinTool::AppInfo app;
	app.AutoBoot = true;
	app.Comments = L"这是一款右键菜单扩展工具";
	app.HelpLink = "www.baidu.com";
	app.URLInfoAbout = "www.google.com";
	app.Publisher = L"小林工作室出品";
	app.DesktopLink = true;
	app.DisplayName = L"小林工具箱";
	app.DisplayVersion = "1.0.0.0";
	app.PragmaFile = "D:\\Program Files\\ShellExt\\setup.exe";
	app.UninstallString = "D:\\Program Files\\ShellExt\\setup.exe -un ";

	//注册产品
	WinTool::RegisterApp(app);

	//获取版本信息
	auto a = WinTool::GetAppValue("setup", "DisplayVersion");
	//修改版本信息
	auto b = WinTool::SetAppValue("setup", "DisplayVersion", "1.2.0.0");
	//再次获取版本信息
	auto c = WinTool::GetAppValue("setup", "DisplayVersion");

	//删除程序注册信息
	WinTool::UnRegisterApp("setup");

	while (true)
	{
		auto id = GetMotherboardID();
		auto id2 = GetDeviceUUID();
	}

	auto data = WinTool::ExecuteCMD("fh_loader.exe");

	::MessageBox(0, data.unicode().c_str(), L"结果", MB_OK);
	return 0;


	WinTool::GetComputerID();

	while (true)
	{
		WinTool::ExecuteCMD("adb devices");
	}

	//auto ret = WinTool::ExecuteCMD("fastboot devices");
	/*std::fstream file("D:/Android_Pad_File/tools/TIK-5-169-win/unpack/system_a.img", std::ios::in | std::ios::out | std::ios::binary);
	file.seekp(0xDCC1BFC0);
	std::vector<char> zeroData(64, 0);
	file.write(zeroData.data(), 64);
	auto b=file.good();
	file.close();
	return 0;
*/

	while (true)
	{
		auto apps = WinTool::GetApps();
		for (auto& it : apps) {
			Log::Info(it.first);
			if (it.first.find("USB") != size_t(-1)) {
				break;
			}
		}
	}

	if (!initWMI()) {
		std::cerr << "WMI 初始化失败。" << std::endl;
		return 1;
	}
	for (;;) {
		find(L"fh_loader.exe");
		Sleep(0); // 稍微调大点避免频繁刷WMI
	}
	if (pSvc) {
		pSvc->Release();
	}
	CoUninitialize();
	return 0;
}

