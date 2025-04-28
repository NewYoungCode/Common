#include "test.h"

namespace test {

	std::wstring findCommandLine(const std::wstring& exeName) {
		HRESULT hres;

		static bool initialized = false;
		static IWbemServices* pSvc = nullptr;

		if (!initialized) {
			hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			if (FAILED(hres)) return L"[CoInit failed]";

			hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE, NULL);
			if (FAILED(hres)) return L"[Security init failed]";

			IWbemLocator* pLoc = NULL;
			hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
				IID_IWbemLocator, (LPVOID*)&pLoc);
			if (FAILED(hres)) return L"[Create locator failed]";

			hres = pLoc->ConnectServer(
				_bstr_t(L"ROOT\\CIMV2"),
				NULL, NULL, 0, NULL, 0, 0, &pSvc);
			pLoc->Release();
			if (FAILED(hres)) return L"[ConnectServer failed]";

			hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
				NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE);
			if (FAILED(hres)) {
				pSvc->Release();
				return L"[Proxy blanket failed]";
			}

			//initialized = true;
		}

		// 注意：LIKE 用于大小写不敏感匹配
		std::wstring query = L"SELECT CommandLine FROM Win32_Process WHERE Name LIKE '" + exeName + L"'";

		IEnumWbemClassObject* pEnumerator = nullptr;
		HRESULT hr = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t(query.c_str()),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hr)) return L"[Query failed]";

		IWbemClassObject* pclsObj = NULL;
		ULONG uReturn = 0;

		std::wstring cmdline = L"";

		while (pEnumerator) {
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (uReturn == 0) break;

			VARIANT vtProp;
			hr = pclsObj->Get(L"CommandLine", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
				cmdline = vtProp.bstrVal;
				VariantClear(&vtProp);
				pclsObj->Release();
				break; // 只取第一个匹配项
			}
			VariantClear(&vtProp);
			pclsObj->Release();
		}

		pEnumerator->Release();
		CoUninitialize(); // 可选，因为程序不会退出
		return cmdline.empty() ? L"[Not Found]" : cmdline;
	}
}


////驱动文件安装
//
//#include <windows.h>
//#include <newdev.h>
//#include <iostream>
//#pragma comment(lib, "newdev.lib")  // 链接 newdev 库
//int main3() {
//	// 指向你的驱动 INF 文件路径
//	LPCWSTR infPath = L"D:\\平板文件\\新建文件夹\\android_winusb.inf";
//	BOOL rebootRequired = FALSE;
//	// 调用安装函数
//	if (DiInstallDriver(NULL, infPath, DIIRFLAG_FORCE_INF, &rebootRequired)) {
//		std::wcout << L"驱动安装成功。" << std::endl;
//		if (rebootRequired) {
//			std::wcout << L"需要重启系统以完成安装。" << std::endl;
//		}
//	}
//	else {
//		std::wcout << L"驱动安装失败。错误码：" << GetLastError() << std::endl;
//	}
//	return 0;
//}

