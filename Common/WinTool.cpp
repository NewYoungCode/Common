#include "WinTool.h"

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

	std::string GetComputerID()
	{
		auto cpuId = GetCPUSerialNumber();
		auto biosId = GetBiosUUID();
		auto mac = GetMacAddress();
		Text::Utf8String u8Str = biosId + "_" + cpuId + "_" + mac;
		u8Str = HttpUtility::Md5Encode(u8Str);
		return u8Str;
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


	std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::Utf8String& _proccname) {

		std::vector<PROCESSENTRY32W> infos;
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		PROCESSENTRY32W pe;
		pe.dwSize = sizeof(PROCESSENTRY32W);
		for (auto status = ::Process32FirstW(hSnapshot, &pe); status != FALSE; status = ::Process32NextW(hSnapshot, &pe)) {
			pe.dwSize = sizeof(PROCESSENTRY32W);
			Text::Utf8String item = pe.szExeFile;
			//不传进程名称查询所有
			if (_proccname.empty()) {
				infos.push_back(pe);
			}
			else {
				if (item == _proccname) {
					infos.push_back(pe);
				}
			}
			//printf("%s %d\n", item.data(),pe.th32processId);
		}
		CloseHandle(hSnapshot);
		return infos;
	}
	std::vector<DWORD> FindProcessId(const Text::Utf8String& _proccname)
	{
		std::vector<DWORD> processIds;
		auto list = FindProcessInfo(_proccname);
		for (auto& it : list) {
			processIds.push_back(it.th32ProcessID);
		}
		return processIds;
	}

	HANDLE OpenProcess(const Text::Utf8String& _proccname) {
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

	Text::Utf8String FindProcessFilename(DWORD processId)
	{
		WCHAR buf[MAX_PATH]{ 0 };
		HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
		DWORD result = ::GetModuleFileNameExW(hProcess, NULL, buf, MAX_PATH);
		CloseHandle(hProcess);
		return buf;
	}
	int CloseProcess(const std::vector<DWORD>& processIds) {
		size_t count = 0;
		for (auto item : processIds) {
			count += CloseProcess(item);
		}
		return count;
	}
	bool CloseProcess(DWORD processId)
	{
		HANDLE bExitCode = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE
			| PROCESS_ALL_ACCESS, FALSE, processId);
		if (bExitCode)
		{
			BOOL bFlag = ::TerminateProcess(bExitCode, 0);
			CloseHandle(bExitCode);
			return true;
		}
		return false;
	}
	bool GetAutoBootStatus(const Text::Utf8String& filename) {
		Text::Utf8String bootstart = filename.empty() ? Path::StartFileName() : filename;
		Text::Utf8String appName = Path::GetFileNameWithoutExtension(bootstart);
		bool bResult = false;
		HKEY subKey;
		if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\\", NULL, KEY_ALL_ACCESS, &subKey))
		{
			return bResult;
		}
		//3、判断注册表项是否已经存在
		WCHAR wBuff[MAX_PATH]{ 0 };
		DWORD nLength = MAX_PATH;
		LSTATUS status = RegGetValueW(subKey, NULL, appName.utf16().c_str(), REG_SZ, NULL, wBuff, &nLength);
		if (status != ERROR_SUCCESS) {
			Text::Utf8String strDir = wBuff;
			if (Path::GetFileName(strDir) == Path::GetFileName(bootstart)) {
				bResult = true;
			}
		}
		::RegCloseKey(subKey);
		return bResult;
	}
	bool SetAutoBoot(const Text::Utf8String& filename, bool bStatus)
	{
		Text::Utf8String bootstart = filename.empty() ? Path::StartFileName() : filename;
		Text::Utf8String appName = Path::GetFileNameWithoutExtension(bootstart);
		HKEY subKey;
		bool bResult = false;
		if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\\", NULL, KEY_ALL_ACCESS, &subKey))
		{
			return false;
		}
		do
		{
			if (bStatus == true)
			{
				auto wStr = bootstart.utf16();
				if (ERROR_SUCCESS == ::RegSetValueExW(subKey, appName.utf16().c_str(), NULL, REG_SZ, (PBYTE)wStr.c_str(), wStr.size() * 2))
				{
					bResult = true;
					break;
				}
			}
			else
			{
				if (ERROR_SUCCESS == ::RegDeleteValueW(subKey, appName.utf16().c_str()))
				{
					bResult = true;
					break;
				}
			}
		} while (false);
		::RegCloseKey(subKey);
		return bResult;
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
	bool CreateDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName, const Text::Utf8String& cmdline, const Text::Utf8String& iconFilename) {
		HRESULT hr = CoInitialize(NULL);
		bool bResult = false;
		if (SUCCEEDED(hr))
		{
			IShellLink* pShellLink;
			hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
			if (SUCCEEDED(hr))
			{
				pShellLink->SetPath(pragmaFilename.utf16().c_str());
				pShellLink->SetWorkingDirectory(Path::GetDirectoryName(pragmaFilename).utf16().c_str());
				pShellLink->SetArguments(cmdline.utf16().c_str());
				if (!iconFilename.empty())
				{
					pShellLink->SetIconLocation(iconFilename.utf16().c_str(), 0);
				}
				IPersistFile* pPersistFile;
				hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
				if (SUCCEEDED(hr))
				{
					WCHAR buf[MAX_PATH]{ 0 };
					SHGetSpecialFolderPathW(0, buf, CSIDL_DESKTOPDIRECTORY, 0);//获取当前用户桌面路径
					Text::Utf8String userDesktop(buf);
					if (!LnkName.empty()) {
						userDesktop += "\\" + LnkName + ".lnk";
					}
					else {
						userDesktop += "\\" + Path::GetFileNameWithoutExtension(pragmaFilename) + ".lnk";
					}
					//设置快捷方式地址
					File::Delete(userDesktop);//删除旧的
					hr = pPersistFile->Save(userDesktop.utf16().c_str(), FALSE);
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
	void DeleteDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName) {
		WCHAR buf[MAX_PATH]{ 0 };
		SHGetSpecialFolderPathW(0, buf, CSIDL_DESKTOPDIRECTORY, 0);//获取当前用户桌面路径
		Text::Utf8String userDesktop(buf);
		if (!LnkName.empty()) {
			userDesktop += "\\" + LnkName + ".lnk";
		}
		else {
			userDesktop += "\\" + Path::GetFileNameWithoutExtension(pragmaFilename) + ".lnk";
		}
		//设置快捷方式地址
		File::Delete(userDesktop);//删除旧的
	}

	LSTATUS RegSetSoftware(HKEY hKey, const Text::Utf8String& regKey, const Text::Utf8String& regValue) {
		if (!regValue.empty()) {
			auto wStr = regValue.utf16();
			return RegSetValueExW(hKey, regKey.utf16().c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(wStr.c_str()), wStr.size() * 2);
		}
		return -1;
	}

	void UnRegisterSoftware(const Text::Utf8String& appName_en) {
		Text::Utf8String regKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
		regKeyPath.append("\\" + appName_en);
		HKEY subKey;
		if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, regKeyPath.utf16().c_str(), NULL, KEY_ALL_ACCESS, &subKey)) {
			return;
		}
		::RegDeleteValueW(subKey, L"DisplayName");
		::RegDeleteValueW(subKey, L"DisplayVersion");
		::RegDeleteValueW(subKey, L"Publisher");
		::RegDeleteValueW(subKey, L"DisplayIcon");
		::RegDeleteValueW(subKey, L"UninstallString");
		::RegDeleteValueW(subKey, L"InstallLocation");
		// 关闭注册表键
		RegCloseKey(subKey);
	}

	bool RegisterSoftware(const AppInfo& appInfo)
	{
		Text::Utf8String regKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
		regKeyPath.append("\\" + Path::GetFileNameWithoutExtension(appInfo.StartLocation));
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

		if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, regKeyPath.utf16().c_str(), NULL, KEY_ALL_ACCESS, &hKey) || \
			ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER, regKeyPath.utf16().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &attr, &hKey, NULL)) {
			std::cout << "SUCCESS" << std::endl;
		}
		else {
			return false;
		}

		RegSetSoftware(hKey, L"DisplayName", appInfo.DisplayName);
		RegSetSoftware(hKey, L"DisplayVersion", appInfo.DisplayVersion);
		RegSetSoftware(hKey, L"DisplayIcon", "\"" + appInfo.StartLocation + "\"");//安装位置
		RegSetSoftware(hKey, L"Publisher", appInfo.DisplayAuthor);
		RegSetSoftware(hKey, L"UninstallString", appInfo.UninstallString);
		RegSetSoftware(hKey, L"InstallLocation", Path::GetDirectoryName(appInfo.StartLocation));//安装位置

		if (appInfo.DesktopLnk) {
			WinTool::CreateDesktopLnk(appInfo.StartLocation, appInfo.DisplayName);
		}
		if (appInfo.AutoBoot) {
			WinTool::SetAutoBoot(appInfo.StartLocation, true);
		}
		else {
			WinTool::SetAutoBoot(appInfo.StartLocation, false);
		}
		// 关闭注册表键
		RegCloseKey(hKey);
		return true;
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
	double GetDiskFreeSize(const Text::Utf8String& path) {
		ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
		std::wstring wStr = path.utf16();
		if (wStr.size() > 0) {
			std::wstring drive = path.utf16().substr(0, 1); // 设置你要查询的磁盘路径
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

	Text::Utf8String ExecuteCmdLine(const Text::Utf8String& cmdStr) {
		HANDLE hReadPipe = NULL; //读取管道
		HANDLE hWritePipe = NULL; //写入管道	
		PROCESS_INFORMATION pi{ 0 }; //进程信息	
		STARTUPINFO	si{ 0 };	//控制命令行窗口信息
		SECURITY_ATTRIBUTES sa{ 0 }; //安全属性
		pi.hProcess = NULL;
		pi.hThread = NULL;
		si.cb = sizeof(STARTUPINFO);
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		char* szBuff = NULL;
		do
		{
			//1.创建管道
			if (!::CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
				break;
			}
			//2.设置命令行窗口的信息为指定的读写管道
			::GetStartupInfoW(&si);
			si.hStdError = hWritePipe;
			si.hStdOutput = hWritePipe;
			si.wShowWindow = SW_HIDE; //隐藏命令行窗口
			si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
			//3.创建获取命令行的进程
			if (!::CreateProcessW(NULL, (LPWSTR)cmdStr.utf16().c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
				break;
			}
			//4.等待读取返回的数据
			::WaitForSingleObject(pi.hProcess, 1000 * 60);//等一分钟
			size_t buffSize = ::GetFileSize(hReadPipe, NULL);
			szBuff = new char[buffSize + 1] { 0 };
			unsigned long size = 0;
			if (!ReadFile(hReadPipe, szBuff, buffSize + 1, &size, 0)) {
				break;
			}
		} while (false);
		//清理工作
		if (hWritePipe) {
			CloseHandle(hWritePipe);
		}
		if (hReadPipe) {
			CloseHandle(hReadPipe);
		}
		if (pi.hProcess) {
			CloseHandle(pi.hProcess);
		}
		if (pi.hThread) {
			CloseHandle(pi.hThread);
		}
		Text::Utf8String outResult(szBuff);
		if (szBuff) {
			delete[] szBuff;
		}
		return outResult;
	}
	Text::Utf8String GetBiosUUID() {
		Text::Utf8String resultStr = ExecuteCmdLine("wmic csproduct get UUID");
		resultStr = resultStr.Replace("UUID", "");
		resultStr = resultStr.Replace(" ", "");
		resultStr = resultStr.Replace("\r", "");
		resultStr = resultStr.Replace("\n", "");
		return resultStr;
	}
	Text::Utf8String GetCPUSerialNumber() {
		Text::Utf8String resultStr = ExecuteCmdLine("wmic cpu get ProcessorId");
		resultStr = resultStr.Replace("ProcessorId", "");
		resultStr = resultStr.Replace(" ", "");
		resultStr = resultStr.Replace("\r", "");
		resultStr = resultStr.Replace("\n", "");
		return resultStr;
	}
	Text::Utf8String GetDiskSerialNumber() {
		Text::Utf8String resultStr = ExecuteCmdLine("wmic diskdrive get SerialNumber");
		resultStr = resultStr.Replace("SerialNumber", "");
		resultStr = resultStr.Replace(" ", "");
		resultStr = resultStr.Replace("\r", "");
		resultStr = resultStr.Replace("\n", "");
		return resultStr;
	}
	Text::Utf8String GetMacAddress()
	{
		std::vector<MyAdpterInfo> adpterInfo;
		GetAdptersInfo(adpterInfo);
		return adpterInfo.size() > 0 ? adpterInfo[0].MacAddress : "";
	}

	Text::Utf8String GetWinVersion()
	{
		Text::Utf8String vname = "UnKnow";
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

	Text::Utf8String ShowFolderDialog(HWND ownerWnd, Text::Utf8String defaultPath, Text::Utf8String title) {
		WCHAR selectedPath[MAX_PATH]{ 0 };
		BROWSEINFOW browseInfo{ 0 };
		browseInfo.hwndOwner = ownerWnd;
		browseInfo.pszDisplayName = selectedPath;
		auto wTitle = title.utf16();
		browseInfo.lpszTitle = wTitle.c_str();
		//设置根目录
		LPITEMIDLIST pidlRoot;
		::SHParseDisplayName(defaultPath.utf16().c_str(), NULL, &pidlRoot, 0, NULL);
		browseInfo.pidlRoot = pidlRoot;
		browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		LPITEMIDLIST itemIdList = SHBrowseForFolderW(&browseInfo);
		if (itemIdList != nullptr) {
			SHGetPathFromIDListW(itemIdList, selectedPath);//设置路径
			CoTaskMemFree(itemIdList);//清理
			return selectedPath;
		}
		return selectedPath;
	}

}