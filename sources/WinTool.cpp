#include "WinTool.h"

#include <ShlObj.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include <psapi.h>
#include <process.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")

#include <windows.h>
#include <taskschd.h>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "ole32.lib")

#include "FileSystem.h"
#include "Util.h"
#include "Time.hpp"
#include "base64.h"

namespace WinTool {
#ifndef FormatError
#define FormatError(x)	(x)
#endif // FormatError
	// 获取系统信息
	void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
	{
		if (NULL == lpSystemInfo)
		{
			return;
		}
		typedef VOID(WINAPI* FuncGetSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
		FuncGetSystemInfo funcGetNativeSystemInfo = (FuncGetSystemInfo)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetNativeSystemInfo");
		// 优先使用 "GetNativeSystemInfo" 函数来获取系统信息
		// 函数 "GetSystemInfo" 存在系统位数兼容性问题
		if (NULL != funcGetNativeSystemInfo)
		{
			funcGetNativeSystemInfo(lpSystemInfo);
		}
		else
		{
			GetSystemInfo(lpSystemInfo);
		}
	}

	// 获取操作系统位数
	int GetSystemBits()
	{
		SYSTEM_INFO si;
		SafeGetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		{
			return 64;
		}
		return 86;
	}

	Text::String GetComputerID() {
		HRESULT hres;

		// 初始化 COM
		hres = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hres)) return "";

		// 设置默认安全性
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
		hres = CoCreateInstance(
			CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID*)&pLoc);
		if (FAILED(hres)) {
			CoUninitialize();
			return "";
		}

		IWbemServices* pSvc = nullptr;
		hres = pLoc->ConnectServer(
			_bstr_t(L"ROOT\\CIMV2"),
			NULL, NULL, 0, NULL, 0, 0, &pSvc);
		if (FAILED(hres)) {
			pLoc->Release();
			CoUninitialize();
			return "";
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
			return "";
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
			return "";
		}

		IWbemClassObject* pClassObject = nullptr;
		ULONG uReturn = 0;
		std::string serialNumber = "";

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
		return Util::MD5FromString(serialNumber);
	}

	Text::String GetUserName()
	{
		WCHAR user[MAX_PATH]{ 0 };
		DWORD len = MAX_PATH;
		::GetUserNameW(user, &len);
		return user;
	}

	Text::String GetComputerName()
	{
		WCHAR computerName[MAX_PATH]{ 0 };
		DWORD size = MAX_PATH;
		::GetComputerNameW(computerName, &size);
		return computerName;
	}

	DWORD GetCurrentProcessId() {
		return ::getpid();
	}
	HWND FindMainWindow(DWORD processId)
	{
		handle_data data;
		data.processId = processId;
		data.best_handle = 0;
		EnumWindows([](HWND handle, LPARAM lParam)->BOOL {
			handle_data& data = *(handle_data*)lParam;
			HWND hwnd = ::GetWindow(handle, GW_OWNER);
			data.best_handle = handle;
			//unsigned long processId = 0;
		//::GetWindowThreadProcessId(handle, &processId);
		//if (data.processId != processId || !IsMainWindow(handle)) {
		//	return TRUE;
		//}
		//data.best_handle = handle;
			return false;
			}, (LPARAM)&data);
		return data.best_handle;
	}


	std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::String& _proccname) {

		std::vector<PROCESSENTRY32W> infos;
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		PROCESSENTRY32W pe;
		pe.dwSize = sizeof(PROCESSENTRY32W);
		for (auto status = ::Process32FirstW(hSnapshot, &pe); status != FALSE; status = ::Process32NextW(hSnapshot, &pe)) {
			pe.dwSize = sizeof(PROCESSENTRY32W);
			Text::String item = pe.szExeFile;
			//不传进程名称查询所有
			if (_proccname.empty()) {
				infos.push_back(pe);
			}
			else {
				if (item.toLower() == _proccname.toLower()) {
					infos.push_back(pe);
				}
			}
			//printf("%s %d\n", item.data(),pe.th32processId);
		}
		CloseHandle(hSnapshot);
		return infos;
	}
	std::vector<DWORD> FindProcessId(const Text::String& _proccname)
	{
		std::vector<DWORD> processIds;
		auto list = FindProcessInfo(_proccname);
		for (auto& it : list) {
			processIds.push_back(it.th32ProcessID);
		}
		return processIds;
	}

	HANDLE OpenProcess(const Text::String& _proccname) {
		std::vector<DWORD> processIds;
		auto list = FindProcessInfo(_proccname);
		if (list.size() > 0) {
			HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, list.at(0).th32ProcessID);
			return hProcess;
		}
		return NULL;
	}

	bool Is86BitPorcess(DWORD processId) {

		return !Is64BitPorcess(processId);
	}

	bool Is64BitPorcess(DWORD processId)
	{
		BOOL bIsWow64 = FALSE;
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
		if (hProcess)
		{
			typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
			LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
			if (NULL != fnIsWow64Process)
			{
				fnIsWow64Process(hProcess, &bIsWow64);
			}
		}
		CloseHandle(hProcess);
		return !bIsWow64;
	}

	Text::String FindProcessFilename(DWORD processId)
	{
		WCHAR buf[MAX_PATH]{ 0 };
		HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
		DWORD result = ::GetModuleFileNameExW(hProcess, NULL, buf, MAX_PATH);
		CloseHandle(hProcess);
		return buf;
	}
	int CloseProcess(const std::vector<DWORD>& processIds) {
		int count = 0;
		for (auto item : processIds) {
			count += CloseProcess(item);
		}
		return count;
	}
	bool CloseProcess(DWORD processId)
	{
		HANDLE handle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE
			| PROCESS_ALL_ACCESS, FALSE, processId);
		if (handle)
		{
			BOOL bFlag = ::TerminateProcess(handle, 0);
			CloseHandle(handle);
			return true;
		}
		return false;
	}
	bool CloseProcess(HANDLE hProcess, UINT exitCode)
	{
		return TerminateProcess(hProcess, exitCode);
	}
	bool IsAutoBoot(const Text::String& _keyName, HKEY rootKey) {
		std::wstring keyName = _keyName.unicode();
		const wchar_t* regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

		HKEY hSubKey = nullptr;
		if (RegOpenKeyExW(rootKey, regPath, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
			WCHAR wBuff[MAX_PATH]{ 0 };
			DWORD size = sizeof(wBuff);
			if (RegGetValueW(hSubKey, nullptr, keyName.c_str(), RRF_RT_REG_SZ, nullptr, wBuff, &size) == ERROR_SUCCESS) {
				Text::String value = wBuff;
				RegCloseKey(hSubKey);
				return true;  // 找到了匹配的启动项
			}
			RegCloseKey(hSubKey);
		}
		return false;
	}

	bool SetAutoBoot(const Text::String& filename, bool bStatus, HKEY rootKey) {
		Text::String fileName_ = filename.empty() ? Path::StartFileName().unicode() : filename.unicode();
		fileName_ = fileName_.replace("/", "\\", true);//windows仅支持\\这种斜杠

		std::wstring fileName = fileName_.unicode();
		std::wstring keyName = Path::GetFileNameWithoutExtension(fileName).unicode();
		const wchar_t* regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

		bool success = false;
		HKEY hSubKey = nullptr;
		if (RegOpenKeyExW(rootKey, regPath, 0, KEY_SET_VALUE, &hSubKey) == ERROR_SUCCESS) {
			if (bStatus) {
				//设置自启动
				if (RegSetValueExW(hSubKey, keyName.c_str(), 0, REG_SZ,
					(const BYTE*)fileName.c_str(), (fileName.size() + 1) * sizeof(wchar_t)) == ERROR_SUCCESS) {
					success = true;
				}
			}
			else {
				//取消自启动
				if (RegDeleteValueW(hSubKey, keyName.c_str()) == ERROR_SUCCESS) {
					success = true;
				}
			}
			RegCloseKey(hSubKey);
		}
		return success;
	}

	bool IsInTask(const Text::String& _taskName) {
		std::wstring taskName = _taskName.unicode();
		bool success = false;
		HRESULT hr = S_OK;
		ITaskService* pService = nullptr;
		ITaskFolder* pRootFolder = nullptr;
		IRegisteredTask* pTask = nullptr;

		do {
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			if (FAILED(hr)) break;

			hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE, NULL);
			if (FAILED(hr)) break;

			hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
				IID_ITaskService, (void**)&pService);
			if (FAILED(hr) || !pService) break;

			hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
			if (FAILED(hr)) break;

			hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
			if (FAILED(hr)) break;

			hr = pRootFolder->GetTask(_bstr_t(taskName.c_str()), &pTask);
			if (SUCCEEDED(hr) && pTask) {
				success = true;
			}
		} while (false);

		if (pTask) pTask->Release();
		if (pRootFolder) pRootFolder->Release();
		if (pService) pService->Release();
		CoUninitialize();
		return success;
	}
	bool AddBootTask(const Text::String& _taskName, const Text::String& _exeFile) {

		std::wstring taskName = _taskName.unicode();
		std::wstring exeFile = _exeFile.unicode();

		bool success = false;
		HRESULT hr = S_OK;

		ITaskService* pService = nullptr;
		ITaskFolder* pRootFolder = nullptr;
		ITaskDefinition* pTask = nullptr;
		IPrincipal* pPrincipal = nullptr;
		ITriggerCollection* pTriggerCollection = nullptr;
		ITrigger* pTrigger = nullptr;
		IActionCollection* pActionCollection = nullptr;
		IAction* pAction = nullptr;
		IExecAction* pExecAction = nullptr;
		IRegisteredTask* pRegisteredTask = nullptr;

		do {
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			if (FAILED(hr)) break;

			hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE, NULL);
			if (FAILED(hr)) break;

			hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
				IID_ITaskService, (void**)&pService);
			if (FAILED(hr) || !pService) break;

			hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
			if (FAILED(hr)) break;

			hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
			if (FAILED(hr)) break;

			// 删除旧任务（忽略错误）
			pRootFolder->DeleteTask(_bstr_t(taskName.c_str()), 0);

			hr = pService->NewTask(0, &pTask);
			if (FAILED(hr)) break;

			hr = pTask->get_Principal(&pPrincipal);
			if (FAILED(hr)) break;
			pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
			pPrincipal->put_LogonType(TASK_LOGON_SERVICE_ACCOUNT);
			pPrincipal->put_UserId(_bstr_t(L"SYSTEM"));

			hr = pTask->get_Triggers(&pTriggerCollection);
			if (FAILED(hr)) break;

			hr = pTriggerCollection->Create(TASK_TRIGGER_BOOT, &pTrigger);
			if (FAILED(hr)) break;

			hr = pTask->get_Actions(&pActionCollection);
			if (FAILED(hr)) break;

			hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
			if (FAILED(hr)) break;

			hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
			if (FAILED(hr)) break;

			hr = pExecAction->put_Path(_bstr_t(exeFile.c_str()));
			if (FAILED(hr)) break;

			hr = pRootFolder->RegisterTaskDefinition(
				_bstr_t(taskName.c_str()),
				pTask,
				TASK_CREATE_OR_UPDATE,
				_variant_t(L"SYSTEM"),
				_variant_t(),
				TASK_LOGON_SERVICE_ACCOUNT,
				_variant_t(L""),
				&pRegisteredTask);
			if (FAILED(hr)) break;

			success = true;
		} while (false);

		if (pRegisteredTask) pRegisteredTask->Release();
		if (pExecAction) pExecAction->Release();
		if (pAction) pAction->Release();
		if (pActionCollection) pActionCollection->Release();
		if (pTrigger) pTrigger->Release();
		if (pTriggerCollection) pTriggerCollection->Release();
		if (pPrincipal) pPrincipal->Release();
		if (pTask) pTask->Release();
		if (pRootFolder) pRootFolder->Release();
		if (pService) pService->Release();
		CoUninitialize();
		return success;
	}


	BOOL EnablePrivilege(HANDLE process)
	{
		// 得到令牌句柄
		HANDLE hToken = NULL;
		bool bResult = false;
		if (OpenProcessToken(process ? process : ::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken)) {
			// 得到特权值
			LUID luid;
			if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
				// 提升令牌句柄权限
				TOKEN_PRIVILEGES tp = {};
				tp.PrivilegeCount = 1;
				tp.Privileges[0].Luid = luid;
				tp.Privileges[0].Attributes = 1 ? SE_PRIVILEGE_ENABLED : 0;
				if (AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL)) {
					bResult = true;
				}
			}
		}
		// 关闭令牌句柄
		CloseHandle(hToken);
		return bResult;
	}

	bool CreateLink(const Text::String& pragmaFilename, const Text::String& linkDir, const Text::String& LnkName, const Text::String& cmdline, const Text::String& iconFilename) {
		if (linkDir.empty() || pragmaFilename.empty()) {
			wprintf(L"CreateLink失败!\n");
			return false;
		}
		HRESULT hr = CoInitialize(NULL);
		bool bResult = false;
		if (SUCCEEDED(hr))
		{
			IShellLink* pShellLink;
			hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
			if (SUCCEEDED(hr))
			{
				pShellLink->SetPath(pragmaFilename.unicode().c_str());
				pShellLink->SetWorkingDirectory(Path::GetDirectoryName(pragmaFilename).unicode().c_str());
				pShellLink->SetArguments(cmdline.unicode().c_str());
				if (!iconFilename.empty())
				{
					pShellLink->SetIconLocation(iconFilename.unicode().c_str(), 0);
				}
				IPersistFile* pPersistFile;
				hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
				if (SUCCEEDED(hr))
				{
					Text::String userDesktop(linkDir);
					if (!LnkName.empty()) {
						userDesktop += "\\" + LnkName + ".lnk";
					}
					else {
						userDesktop += "\\" + Path::GetFileNameWithoutExtension(pragmaFilename) + ".lnk";
					}
					//设置快捷方式地址
					File::Delete(userDesktop);//删除旧的
					hr = pPersistFile->Save(userDesktop.unicode().c_str(), FALSE);
					if (SUCCEEDED(hr))
					{
						bResult = true;
					}
					pPersistFile->Release();
				}
				pShellLink->Release();
			}
		}
		CoUninitialize();
		return bResult;
	}

	void DeleteLink(const Text::String& linkDir, const Text::String& pragmaFilename, const Text::String& LnkName) {
		if (linkDir.empty()) {
			return;
		}
		Text::String lnkFile = linkDir;
		if (!LnkName.empty()) {
			lnkFile += "\\" + LnkName + ".lnk";
		}
		else {
			lnkFile += "\\" + Path::GetFileNameWithoutExtension(pragmaFilename) + ".lnk";
		}
		File::Delete(lnkFile);//删除快捷方式
	}

	void DeleteKeyRecursively(HKEY hKeyParent, const wchar_t* subKey) {
		HKEY hKey;
		LONG result = ::RegOpenKeyExW(hKeyParent, subKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE, &hKey);
		if (result != ERROR_SUCCESS) {
			std::wcerr << L"Failed to open registry key for deletion. Error: " << result << std::endl;
			return;
		}

		// 获取子项的数量
		DWORD dwIndex = 0;
		FILETIME ftLastWriteTime;
		WCHAR szSubKey[MAX_PATH];
		DWORD dwSubKeyLen;

		while (true) {
			dwSubKeyLen = sizeof(szSubKey) / sizeof(szSubKey[0]);
			result = ::RegEnumKeyExW(hKey, dwIndex, szSubKey, &dwSubKeyLen, nullptr, nullptr, nullptr, &ftLastWriteTime);
			if (result == ERROR_NO_MORE_ITEMS) {
				break; // 没有更多子项，退出循环
			}

			if (result == ERROR_SUCCESS) {
				// 递归删除子项
				DeleteKeyRecursively(hKey, szSubKey);
				++dwIndex; // 移动到下一个子项
			}
			else {
				std::wcerr << L"Failed to enumerate subkey. Error: " << result << std::endl;
				break;
			}
		}

		// 删除当前项
		result = ::RegDeleteKeyW(hKeyParent, subKey);
		if (result != ERROR_SUCCESS) {
			std::wcerr << L"Failed to delete registry key. Error: " << result << std::endl;
		}
		else {
			std::wcout << L"Deleted registry key: " << subKey << std::endl;
		}

		::RegCloseKey(hKey);
	}

#if defined(_WIN64)
	const Text::String g_regKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
#else
	const Text::String g_regKeyPath = L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
#endif

	LSTATUS RegSetSoftware(HKEY hKey, const Text::String& regKey, const Text::String& regValue) {
		if (!regValue.empty()) {
			auto wStr = regValue.unicode();
			return RegSetValueExW(hKey, regKey.unicode().c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(wStr.c_str()), wStr.size() * 2);
		}
		return -1;
	}
	bool SetAppValue(const Text::String& appName_en, const Text::String& key, const Text::String& value) {
		Text::String regKeyPath = g_regKeyPath;
		regKeyPath.append("\\" + appName_en);

		HKEY hKey = nullptr;

		// 如果你是 64 位程序写 HKEY_LOCAL_MACHINE，建议使用 KEY_WOW64_64KEY
		LSTATUS result = RegOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			regKeyPath.unicode().c_str(),
			0,
			KEY_SET_VALUE | KEY_WOW64_64KEY,
			&hKey
		);

		if (result != ERROR_SUCCESS) {
			wprintf(L"打开注册表失败，错误码: %ld\n", result);
			return false;
		}

		result = RegSetSoftware(hKey, key, value);
		RegCloseKey(hKey);

		return (result == ERROR_SUCCESS);
	}

	Text::String GetAppValue(const Text::String& appName_en, const Text::String& key) {

		Text::String regKeyPath = g_regKeyPath;
		regKeyPath.append("\\" + appName_en);

		HKEY hKey = nullptr;
		LSTATUS result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regKeyPath.unicode().c_str(), 0, KEY_READ, &hKey);
		if (result != ERROR_SUCCESS) {
			wprintf(L"打开注册表失败，错误码: %ld\n", result);
			return L""; // 返回空表示获取失败
		}

		wchar_t versionBuffer[MAX_PATH]{};
		DWORD bufferSize = sizeof(versionBuffer);
		DWORD type = 0;

		result = RegQueryValueExW(hKey, key.unicode().c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(versionBuffer), &bufferSize);
		RegCloseKey(hKey);

		if (result == ERROR_SUCCESS && type == REG_SZ) {
			return Text::String(versionBuffer);
		}
		else {
			wprintf(L"读取失败，错误码: %ld\n", result);
		}
		return L"";
	}

	bool RegisterApp(const AppInfo& appInfo)
	{
		Text::String regKeyPath = g_regKeyPath;

		regKeyPath.append("\\" + Path::GetFileNameWithoutExtension(appInfo.PragmaFile));
		// 创建注册表项
		HKEY hKey;

		SECURITY_DESCRIPTOR securityDesc;
		if (!InitializeSecurityDescriptor(&securityDesc, SECURITY_DESCRIPTOR_REVISION))
		{
			DWORD error = GetLastError();
			wprintf(L"无法初始化安全描述符。错误代码：%d\n", error);
			return false;
		}
		if (!SetSecurityDescriptorDacl(&securityDesc, TRUE, NULL, FALSE))
		{
			DWORD error = GetLastError();
			wprintf(L"无法设置DACL。错误代码：%d\n", error);
			return false;
		}
		SECURITY_ATTRIBUTES attr;
		attr.nLength = sizeof(attr);
		attr.lpSecurityDescriptor = &securityDesc;
		attr.bInheritHandle = FALSE;

		if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, regKeyPath.unicode().c_str(), NULL, KEY_ALL_ACCESS, &hKey) || \
			ERROR_SUCCESS == RegCreateKeyExW(HKEY_LOCAL_MACHINE, regKeyPath.unicode().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &attr, &hKey, NULL)) {
			std::cout << "SUCCESS" << std::endl;
		}
		else {
			return false;
		}

		RegSetSoftware(hKey, L"DisplayName", appInfo.DisplayName);
		RegSetSoftware(hKey, L"DisplayVersion", appInfo.DisplayVersion);
		if (!appInfo.DisplayIcon.empty()) {//如果传入了ICON路径
			RegSetSoftware(hKey, L"DisplayIcon", appInfo.DisplayIcon);
		}
		else {
			//不传用exe文件的图标
			RegSetSoftware(hKey, L"DisplayIcon", "\"" + appInfo.PragmaFile + "\"");//安装位置
		}
		RegSetSoftware(hKey, L"Publisher", appInfo.Publisher);
		RegSetSoftware(hKey, L"UninstallString", appInfo.UninstallString);
		RegSetSoftware(hKey, L"PragmaFile", appInfo.PragmaFile);//程序启动路径

		if (!appInfo.InstallLocation.empty()) {
			RegSetSoftware(hKey, L"InstallLocation", appInfo.InstallLocation);//安装位置
		}
		else {
			RegSetSoftware(hKey, L"InstallLocation", Path::GetDirectoryName(appInfo.PragmaFile));//安装位置
		}

		RegSetSoftware(hKey, L"URLInfoAbout", appInfo.URLInfoAbout);
		RegSetSoftware(hKey, L"HelpLink", appInfo.HelpLink);
		RegSetSoftware(hKey, L"InstallDate", Time::Now().ToString("yyyy-MM-dd"));//安装日期
		RegSetSoftware(hKey, L"Comments", appInfo.Comments);
		RegSetSoftware(hKey, L"AllUsers", std::to_string(appInfo.AllUsers));

		//创建开始菜单程序
		CreateLink(appInfo.PragmaFile, Path::StartPrograms(appInfo.AllUsers), appInfo.DisplayName);
		if (appInfo.DesktopLink) {
			//创建桌面快捷方式
			CreateLink(appInfo.PragmaFile, Path::UserDesktop(appInfo.AllUsers), appInfo.DisplayName);
		}
		if (appInfo.AutoBoot) {
			WinTool::SetAutoBoot(appInfo.PragmaFile, true, appInfo.AllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER);
		}
		else {
			WinTool::SetAutoBoot(appInfo.PragmaFile, true, appInfo.AllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER);
		}
		// 关闭注册表键
		RegCloseKey(hKey);
		return true;
	}
	void UnRegisterApp(const Text::String& appName_en) {

		auto DisplayName = GetAppValue(appName_en, "DisplayName");
		auto PragmaFile = GetAppValue(appName_en, "PragmaFile");

		//删除开始菜单快捷方式
		DeleteLink(Path::StartPrograms(true), PragmaFile, DisplayName);
		DeleteLink(Path::StartPrograms(false), PragmaFile, DisplayName);

		//删除桌面快捷方式
		DeleteLink(Path::UserDesktop(true), PragmaFile, DisplayName);
		DeleteLink(Path::UserDesktop(false), PragmaFile, DisplayName);

		Text::String regKeyPath = g_regKeyPath;
		regKeyPath.append("\\" + appName_en);

#if defined(_WIN64)
		// 对于 64 位注册表
		LSTATUS result = RegDeleteKeyExW(
			HKEY_LOCAL_MACHINE,
			regKeyPath.unicode().c_str(),
			KEY_WOW64_64KEY,
			0);
#else
		// 对于 32 位注册表
		LSTATUS result = RegDeleteTreeW(
			HKEY_LOCAL_MACHINE,
			regKeyPath.unicode().c_str());
#endif

		if (result != ERROR_SUCCESS) {
			wprintf(L"删除注册表失败，错误码: %d\n", result);
		}
	}

	//软件许可必要
	const char* licenser = "Y3B1aWQreWFuZ3JlZ2VkaXRcclxu000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	const char* licenser2 = "K3lhbmdyZWdlZGl0";
#define LICENSER_SIZE 256
	bool RegisterLicenser(const Text::String& exeFilename, const Text::String& softwareData) {
		bool ok = false;
		do
		{
			std::string key = base64_decode("WTNCMWFXUXJlV0Z1WjNKbFoyVmthWFJjY2x4dQ==");
			std::string data;
			File::ReadFile(exeFilename, &data);
			size_t pos = data.find(key);
			if (pos != size_t(-1)) {
				char buf[LICENSER_SIZE]{ 0 };
				Text::String newHead = WinTool::GetComputerID() + licenser2 + base64_encode(softwareData);
				::memcpy(buf, newHead.c_str(), newHead.size());
				std::fstream file(exeFilename.unicode(), std::ios::in | std::ios::out | std::ios::binary);
				file.seekp(pos, std::ios::beg);
				file.write(buf, sizeof(buf));
				file.flush();
				file.close();
				ok = file.good();
			}
		} while (false);
		return ok;
	}

	Text::String FindLicenser(const Text::String& exeFilename) {
		std::string data;
		File::ReadFile(exeFilename, &data);
		std::string key = WinTool::GetComputerID() + licenser2;
		size_t pos = data.find(key);
		if (pos != size_t(-1)) {
			data = data.c_str() + pos + key.size();
			return base64_decode(data);
		}
		return "";
	}


	// 头文件包含
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <WinSock.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <iostream>
#pragma comment(lib,"iphlpapi.lib")
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
	int GetAdptersInfo(std::vector<MyAdpterInfo>& adpterInfo)
	{
		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;
		DWORD dwRetVal = 0;
		UINT i;

		/* variables used to print DHCP time info */
		struct tm newtime;
		char buffer[32];
		errno_t error = 0;

		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
		pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
		if (pAdapterInfo == NULL)
		{
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			return -1;
		}
		// Make an initial call to GetAdaptersInfo to get
		// the necessary size into the ulOutBufLen variable
		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			FREE(pAdapterInfo);
			pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
			if (pAdapterInfo == NULL)
			{
				printf("Error allocating memory needed to call GetAdaptersinfo\n");
				return -1;    //    error data return
			}
		}

		if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
		{
			pAdapter = pAdapterInfo;
			while (pAdapter)
			{
				MyAdpterInfo info;
				info.Name = std::string(pAdapter->AdapterName);
				info.Description = std::string(pAdapter->Description);
				info.Type = pAdapter->Type;
				char buffer[20];
				sprintf_s(buffer, "%.2x-%.2x-%.2x-%.2x-%.2x-%.2x", pAdapter->Address[0],
					pAdapter->Address[1], pAdapter->Address[2], pAdapter->Address[3],
					pAdapter->Address[4], pAdapter->Address[5]);
				info.MacAddress = std::string(buffer);
				IP_ADDR_STRING* pIpAddrString = &(pAdapter->IpAddressList);
				do
				{
					info.Ip.push_back(std::string(pIpAddrString->IpAddress.String));
					pIpAddrString = pIpAddrString->Next;
				} while (pIpAddrString);
				pAdapter = pAdapter->Next;
				adpterInfo.push_back(info);
			}
			if (pAdapterInfo)
				FREE(pAdapterInfo);
			return 0;    // normal return
		}
		else
		{
			if (pAdapterInfo)
				FREE(pAdapterInfo);
			printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
			return 1;    //    null data return
		}
	}
	double GetDiskFreeSize(const Text::String& path) {
		ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
		std::wstring wStr = path.unicode();
		if (wStr.size() > 0) {
			std::wstring drive = path.unicode().substr(0, 1); // 设置你要查询的磁盘路径
			drive += L":\\";
			if (GetDiskFreeSpaceExW(drive.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
				double freeSpaceGB = static_cast<double>(freeBytesAvailable.QuadPart) / (1024 * 1024 * 1024);
				return freeSpaceGB;
			}
		}
		return 0;
	};
	void EnCode(const File::FileStream* fileData, File::FileStream* outData) {
		size_t stramSize = fileData->size();
		char* memBytes = new char[stramSize];
		for (size_t i = 0; i < stramSize; i++)
		{
			memBytes[i] = fileData->at(i) + 1;
		}
		outData->clear();
		outData->reserve(stramSize);
		outData->resize(stramSize);
		::memcpy((void*)outData->c_str(), memBytes, stramSize);
		delete[] memBytes;
	}
	void DeCode(const File::FileStream* fileData, File::FileStream* outData) {
		size_t stramSize = fileData->size();
		char* memBytes = new char[stramSize];
		for (size_t i = 0; i < stramSize; i++)
		{
			memBytes[i] = fileData->at(i) - 1;
		}
		outData->clear();
		outData->reserve(stramSize);
		outData->resize(stramSize);
		::memcpy((void*)outData->c_str(), memBytes, stramSize);
		delete[] memBytes;
	}
	Text::String ExecuteCMD(const Text::String& cmdStr, std::function<void(const Text::String&)> callback, HANDLE* outHandle) {
		HANDLE hReadPipe = NULL;
		HANDLE hWritePipe = NULL;
		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFOW si = { 0 };
		SECURITY_ATTRIBUTES sa = { 0 };
		si.cb = sizeof(STARTUPINFOW);
		pi.hProcess = NULL;
		pi.hThread = NULL;
		sa.bInheritHandle = TRUE;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;

		Text::String result;
		std::wstring cmdLine = cmdStr.unicode(); // 防止指针悬挂
		do {
			// 创建匿名管道
			if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
				break;
			}
			//SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0); // 父进程不继承读端
			// 设置标准输出和错误输出重定向到写入管道
			//GetStartupInfoW(&si);//这行代码会导致窗口程序无法获得输出
			si.hStdOutput = hWritePipe;
			si.hStdError = hWritePipe;
			si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;

			// 创建子进程
			if (!CreateProcessW(NULL, (WCHAR*)cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
				break;
			}
			// 可选：返回进程句柄
			if (outHandle) {
				*outHandle = pi.hProcess;
			}

			CloseHandle(hWritePipe); // 必须关闭写入端，避免 ReadFile 阻塞
			hWritePipe = NULL;

			// 开始读取管道内容
			char buffer[512];
			DWORD bytesRead = 0;
			while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
				std::string temp(buffer, bytesRead);
				result.append(temp);
				if (callback) {
					callback(temp);
				}
			}

			// 等待子进程结束（如果未通过 outHandle 返回）
			if (!outHandle) {
				WaitForSingleObject(pi.hProcess, INFINITE);
			}

		} while (false);

		// 清理资源
		if (hWritePipe) {
			CloseHandle(hWritePipe);
		}
		if (hReadPipe) {
			CloseHandle(hReadPipe);
		}
		if (pi.hThread) {
			CloseHandle(pi.hThread);
		}
		if (pi.hProcess) {
			CloseHandle(pi.hProcess);
		}

		return result;
	}

	Text::String GetMacAddress()
	{
		std::vector<MyAdpterInfo> adpterInfo;
		GetAdptersInfo(adpterInfo);
		return adpterInfo.size() > 0 ? adpterInfo[0].MacAddress : "";
	}

	Text::String GetWinVersion()
	{
		Text::String vname = "UnKnow";
		typedef void(WINAPI* NTPROC)(DWORD*, DWORD*, DWORD*);
		HINSTANCE hinst = NULL;;
		DWORD dwMajor, dwMinor, dwBuildNumber;
		NTPROC proc = NULL;
		if ((hinst = ::LoadLibraryW(L"ntdll.dll")) && (proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers"))) {
			proc(&dwMajor, &dwMinor, &dwBuildNumber);
			dwBuildNumber = (DWORD)((LOWORD(dwBuildNumber)));
			if (dwMajor == 10 && dwMinor == 0 && dwBuildNumber >= 22000)	//win 11
			{
				vname = "Windows 11";
			}
			else if (dwMajor == 10 && dwMinor == 0) {
				vname = "Windows 10";
			}
			else if (dwMajor == 6 && dwMinor == 3)
			{
				vname = "Windows 8.1";
			}
			else if (dwMajor == 6 && dwMinor == 2) {
				vname = "Windows 8";
			}
			else if (dwMajor == 6 && dwMinor == 1) {
				vname = "Windows 7";
			}
			vname += "_x" + std::to_string(GetSystemBits()) + " " + std::to_string(dwBuildNumber);
		}
		if (hinst) {
			::FreeLibrary(hinst);
		}
		return vname;
	}

	Text::String ShowFileDialog(HWND ownerWnd, const Text::String& defaultPath, const Text::String& title, const Text::String& filter) {
		OPENFILENAMEW ofn;       // 打开文件对话框结构体
		WCHAR szFile[512]{ 0 };       // 选择的文件名
		// 初始化OPENFILENAME结构体
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.hwndOwner = ownerWnd;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		auto oldWorkDir = Path::StartPath();
		do
		{
			// 显示文件对话框
			if (GetOpenFileNameW(&ofn) == TRUE) {
				break;
			}
		} while (false);
		::SetCurrentDirectoryW(oldWorkDir.unicode().c_str());
		return szFile;
	}

	Text::String ShowFolderDialog(HWND ownerWnd, const Text::String& defaultPath, const Text::String& title) {
		//记录旧的工作目录
		auto oldWorkDir = Path::StartPath();

		WCHAR selectedPath[MAX_PATH]{ 0 };
		BROWSEINFOW browseInfo{ 0 };
		browseInfo.hwndOwner = ownerWnd;
		browseInfo.pszDisplayName = selectedPath;
		auto wTitle = title.unicode();
		browseInfo.lpszTitle = wTitle.c_str();
		//设置根目录
		LPITEMIDLIST pidlRoot;
		::SHParseDisplayName(defaultPath.unicode().c_str(), NULL, &pidlRoot, 0, NULL);
		browseInfo.pidlRoot = pidlRoot;
		browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		LPITEMIDLIST itemIdList = SHBrowseForFolderW(&browseInfo);
		do
		{
			if (itemIdList != nullptr) {
				SHGetPathFromIDListW(itemIdList, selectedPath);//设置路径
				CoTaskMemFree(itemIdList);//清理
				break;
			}
		} while (false);
		//恢复工作目录
		::SetCurrentDirectoryW(oldWorkDir.unicode().c_str());
		return selectedPath;
	}

	std::string GetMacAddress(DWORD ipAddress) {
		BYTE macAddress[6];
		DWORD macAddressLength = sizeof(macAddress);

		DWORD result = SendARP(ipAddress, 0, macAddress, &macAddressLength);
		if (result == NO_ERROR) {
			std::ostringstream oss;
			oss << std::hex << std::setfill('0');

			for (int i = 0; i < 6; ++i) {
				if (i > 0) {
					oss << "-";
				}
				oss << std::setw(2) << static_cast<int>(macAddress[i]);
			}

			return oss.str();
		}
		else {
			return "Failed to get MAC address.";
		}
	}
	RouterInfo GetRouterInfo() {
		RouterInfo info;
		MIB_IPFORWARDTABLE* pForwardTable;
		DWORD dwSize = 0;
		// 获取所需的缓冲区大小
		GetIpForwardTable(nullptr, &dwSize, false);
		// 分配缓冲区
		pForwardTable = (MIB_IPFORWARDTABLE*)malloc(dwSize);
		if (pForwardTable == nullptr) {
			std::cout << "Failed to allocate memory." << std::endl;
			return info;
		}
		// 获取路由表信息
		DWORD result = GetIpForwardTable(pForwardTable, &dwSize, false);
		if (result != NO_ERROR) {
			std::cout << "Failed to get IP forward table. Error code: " << result << std::endl;
			free(pForwardTable);
			return info;
		}
		// 查找默认网关
		for (DWORD i = 0; i < pForwardTable->dwNumEntries; i++) {
			MIB_IPFORWARDROW row = pForwardTable->table[i];
			if (row.dwForwardDest == 0 && row.dwForwardProto == MIB_IPPROTO_NETMGMT) {
				DWORD gatewayIp = row.dwForwardNextHop;
				char ip[256]{ 0 };
				UCHAR ip1 = ((gatewayIp >> 0) & 0xFF);
				UCHAR ip2 = ((gatewayIp >> 8) & 0xFF);
				UCHAR ip3 = ((gatewayIp >> 16) & 0xFF);
				UCHAR ip4 = ((gatewayIp >> 24) & 0xFF);
				sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
				info.IP = ip;
				DWORD ipAddress = ::inet_addr(ip);  // 替换为你想要查询的IP地址
				info.MAC = GetMacAddress(ipAddress);
				info.MAC = info.MAC.toLower();
				break;
			}
		}
		// 释放分配的内存
		free(pForwardTable);
		return info;
	}

#include <setupapi.h>
#include <devguid.h>
#pragma comment(lib, "setupapi.lib")  // 需要链接 setupapi.lib
	std::vector<Text::String> GetComPorts() {
		std::vector<Text::String> comPorts;
		HDEVINFO hDevInfo;
		SP_DEVINFO_DATA DeviceInfoData;
		DWORD i;

		// 获取所有的串口设备
		hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) {
			return comPorts;
		}

		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
			char szFriendlyName[256];
			DWORD DataT;
			DWORD buffersize = sizeof(szFriendlyName);

			// 获取设备的友好名称（Friendly Name），如 "USB-SERIAL CH340 (COM3)"
			if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME,
				&DataT, (PBYTE)szFriendlyName, buffersize, NULL)) {
				std::string name(szFriendlyName);
				size_t pos = name.find("(COM");
				if (pos != std::string::npos) {
					size_t end = name.find(")", pos);
					if (end != std::string::npos) {
						comPorts.push_back(name);
					}
				}
			}
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
		return comPorts;
	}

#include <newdev.h>
#pragma comment(lib, "newdev.lib")  // 链接 newdev 库
	bool InstallDriver(const Text::String& infPath, bool* needReboot) {
		// 指向你的驱动 INF 文件路径
		BOOL rebootRequired = FALSE;
		// 调用安装函数
		if (DiInstallDriver(NULL, infPath.unicode().c_str(), DIIRFLAG_FORCE_INF, &rebootRequired)) {
			std::wcout << L"驱动安装成功。" << std::endl;
			if (rebootRequired) {
				if (needReboot) {
					*needReboot = true;
				}
				std::wcout << L"需要重启系统以完成安装。" << std::endl;
			}
			return true;
		}
		else {
			std::wcout << L"驱动安装失败。错误码：" << GetLastError() << std::endl;
		}
		return false;
	}

	void inline __EnumerateInstalledSoftware(HKEY hRootKey, REGSAM viewFlag, std::map<Text::String, Text::String>& list) {
		HKEY hKey;
		const wchar_t* subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

		if (RegOpenKeyExW(hRootKey, subkey, 0, KEY_READ | viewFlag, &hKey) != ERROR_SUCCESS) {
			return;
		}

		wchar_t keyName[MAX_PATH];
		DWORD keyNameSize;
		DWORD index = 0;

		while (true) {
			keyNameSize = sizeof(keyName) / sizeof(wchar_t);
			FILETIME ftLastWriteTime;

			LONG result = RegEnumKeyExW(
				hKey,
				index,
				keyName,
				&keyNameSize,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime
			);

			if (result == ERROR_NO_MORE_ITEMS) break;
			if (result == ERROR_SUCCESS) {
				HKEY hSubKey;
				if (RegOpenKeyExW(hKey, keyName, 0, KEY_READ | viewFlag, &hSubKey) == ERROR_SUCCESS) {
					wchar_t displayName[512];
					wchar_t installLocation[512];
					DWORD sizeName = sizeof(displayName);
					DWORD sizeLoc = sizeof(installLocation);
					DWORD type = 0;

					if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &type, (LPBYTE)displayName, &sizeName) == ERROR_SUCCESS) {
						if (type == REG_SZ) {
							std::wstring name = displayName;
							std::wstring location = L"";

							if (RegQueryValueExW(hSubKey, L"InstallLocation", NULL, &type, (LPBYTE)installLocation, &sizeLoc) == ERROR_SUCCESS) {
								if (type == REG_SZ) {
									location = installLocation;
								}
							}
							//避免重复
							if (list.find(name) == list.end()) {
								list[name] = location;
							}
						}
					}

					RegCloseKey(hSubKey);
				}
			}
			++index;
		}
		RegCloseKey(hKey);
	}
	std::map<Text::String, Text::String> GetApps() {
		std::map<Text::String, Text::String> list;
		__EnumerateInstalledSoftware(HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY, list);
		__EnumerateInstalledSoftware(HKEY_LOCAL_MACHINE, KEY_WOW64_32KEY, list);
		__EnumerateInstalledSoftware(HKEY_CURRENT_USER, KEY_WOW64_64KEY, list);
		__EnumerateInstalledSoftware(HKEY_CURRENT_USER, KEY_WOW64_32KEY, list);
		return list;
	}

#if 1 //反调试相关代码
	typedef NTSTATUS(NTAPI* _NtQueryInformationProcess)(
		HANDLE           ProcessHandle,
		DWORD ProcessInformationClass,
		PVOID            ProcessInformation,
		ULONG            ProcessInformationLength,
		PULONG           ReturnLength
		);
	bool testBeginDebugged();
	bool testNtGlobalFlag();
	bool testProcessDebugPort();
	bool testNtQueryInformationProcess();
	bool testNtPort(_NtQueryInformationProcess NtQueryInformationProcess);
	bool testNtObjectHandle(_NtQueryInformationProcess NtQueryInformationProcess);
	bool testFlag(_NtQueryInformationProcess NtQueryInformationProcess);
	bool IsExceptionHijacked()
	{
		volatile int triggered = 0;
		__try {
			// 故意触发异常（非法指针解引用）
			volatile int* p = nullptr;
			triggered = *p; // 访问 0 地址
			printf("%d", triggered);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			// 如果进入这里，说明异常被我们程序自己捕获了
			return false; // 未被劫持
		}
		// 如果能走到这里，说明异常没有被捕获，可能被调试器接管了
		return true;
	}

	bool testBeginDebugged()
	{
		if (IsDebuggerPresent())
		{
			//std::cout << "BeginDebugged验证失败，程序被调试" << std::endl;
			return true;
		}
		else
		{
			//std::cout << "BeginDebugged验证正常" << std::endl;
			return false;
		}
	}
	bool testNtGlobalFlag()
	{
		/*DWORD IsDebug = 1;
		__asm
		{
			push eax
			mov eax, fs: [0x30]
			mov eax, [eax + 0x68]
			mov IsDebug, eax
			pop eax
		}
		if (IsDebug == 0x70)
		{
			std::cout << "NtGlobalFlag验证失败，程序被调试" << std::endl;
			return true;
		}
		else
		{
			std::cout << "NtGlobalFlag验证正常" << std::endl;
			return false;

		}*/
		return false;
	}
	bool testProcessDebugPort()
	{
		BOOL IsDebug = FALSE;
		CheckRemoteDebuggerPresent(GetCurrentProcess(), &IsDebug);
		if (IsDebug == TRUE)
		{
			//	std::cout << "ProcessDebugPort验证失败，程序被调试" << std::endl;
			return true;
		}
		else
		{
			//	std::cout << "ProcessDebugPort验证正常，程序未被调试" << std::endl;
			return false;
		}
	}
	bool testNtQueryInformationProcess()
	{
		HMODULE  hDll = LoadLibraryW(L"Ntdll.dll");
		_NtQueryInformationProcess NtQueryInformationProcess = (_NtQueryInformationProcess)GetProcAddress(hDll, "NtQueryInformationProcess");
		auto b1 = testNtPort(NtQueryInformationProcess);
		auto b2 = testNtObjectHandle(NtQueryInformationProcess);
		auto b3 = testFlag(NtQueryInformationProcess);
		if (hDll) {
			::FreeLibrary(hDll);
		}
		return b1 || b2 || b3;
	}
	bool testNtPort(_NtQueryInformationProcess NtQueryInformationProcess)
	{
		HANDLE hProcess = GetCurrentProcess();
		DWORD DebugPort;
		NtQueryInformationProcess(hProcess, 7, &DebugPort, sizeof(DWORD), NULL);
		if (DebugPort != 0)
		{
			//std::cout << "DebugPort验证失败，程序正在被调试" << std::endl;
			return true;
		}
		else
		{
			//std::cout << "DebugPort验证成功" << std::endl;
			return false;
		}
	}
	bool testNtObjectHandle(_NtQueryInformationProcess NtQueryInformationProcess)
	{
		HANDLE hProcess = GetCurrentProcess();
		HANDLE ObjectHandle;
		NtQueryInformationProcess(hProcess, 30, &ObjectHandle, sizeof(ObjectHandle), NULL);
		if (ObjectHandle != NULL)
		{
			//std::cout << "调试端口验证失败，程序正在被调试" << std::endl;
			return true;
		}
		else
		{
			//std::cout << "调试端口验证成功" << std::endl;
			return false;
		}
	}
	bool testFlag(_NtQueryInformationProcess NtQueryInformationProcess)
	{
		HANDLE hProcess = GetCurrentProcess();
		BOOL Flags;
		NtQueryInformationProcess(hProcess, 31, &Flags, sizeof(Flags), NULL);
		if (Flags != 1)
		{
			//std::cout << "调试端口验证失败，程序正在被调试" << std::endl;
			return true;
		}
		else
		{
			//std::cout << "调试端口验证成功" << std::endl;
			return false;
		}
	}
#endif // 0

	bool CheckDebug() {
		auto b1 = testBeginDebugged();
		auto b2 = testProcessDebugPort();
		auto b3 = testNtQueryInformationProcess();
		auto b4 = testNtGlobalFlag();
		return b1 || b2 || b3 || b4;
	}

	bool IsRunning(const Text::String& productName, bool lock)
	{
		Text::String lockFile = Path::GetAppDataPath(productName) + "/__running.lock";
		auto hFile = ::CreateFileW(
			lockFile.unicode().c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,                      // 不允许共享，独占锁定
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		bool isRun = (hFile == INVALID_HANDLE_VALUE);
		// 如果没有锁定并且文件成功打开，关闭文件句柄
		if (!lock && hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
		}
		return isRun;
	}
	void AddFirewallRule(const Text::String& programFile)
	{
		Text::String rullName = Path::GetFileName(programFile) + Util::MD5FromString(programFile);
		//查询规则
		auto ret = WinTool::ExecuteCMD("cmd.exe /c netsh advfirewall firewall show rule name=\"" + rullName + "\"");
		if (ret.find("--------------------------------------") == size_t(-1)) {
			//不存在就添加规则
			Text::String cmd = "cmd.exe /c netsh advfirewall firewall add rule name=\"" + rullName + "\" dir=in action=allow program=\"" + programFile + "\" enable=yes";
			ret = WinTool::ExecuteCMD(cmd);
		}
		//删除规则
		//ret = WinTool::ExecuteCMD("cmd.exe /c netsh advfirewall firewall delete rule name=\"" + rullName + "\"");
	}
}