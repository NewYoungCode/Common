#include "WinTool.h"

namespace WinTool {
#ifndef FormatError
#define FormatError(x)	(x)
#endif // FormatError
	// ��ȡϵͳ��Ϣ
	void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
	{
		if (NULL == lpSystemInfo)
		{
			return;
		}
		typedef VOID(WINAPI* FuncGetSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
		FuncGetSystemInfo funcGetNativeSystemInfo = (FuncGetSystemInfo)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetNativeSystemInfo");
		// ����ʹ�� "GetNativeSystemInfo" ��������ȡϵͳ��Ϣ
		// ���� "GetSystemInfo" ����ϵͳλ������������
		if (NULL != funcGetNativeSystemInfo)
		{
			funcGetNativeSystemInfo(lpSystemInfo);
		}
		else
		{
			GetSystemInfo(lpSystemInfo);
		}
	}

	// ��ȡ����ϵͳλ��
	int GetSystemBits()
	{
		SYSTEM_INFO si;
		SafeGetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		{
			return 64;
		}
		return 32;
	}
	void __Encrypt(void* lpInbuf, DWORD dwInsize, void* lpOutbuf, DWORD* lpOutsize)
	{
		DWORD i;
		for (i = 0; i < dwInsize; i++)
		{
			WORD s = (WORD)(i * i * 7 % 237 + 1);
			*(((WORD*)lpOutbuf) + i) = (*(((BYTE*)lpInbuf) + i) + (BYTE)(i % 255)) & 0xff;
			*(((WORD*)lpOutbuf) + i) *= s;
		}
		*lpOutsize = dwInsize * 2;
	}

	void __Bin2Text(void* lpInbuf, DWORD dwInsize, void* lpOutbuf, DWORD* lpOutsize)
	{
		DWORD i = 0;
		BYTE* tmpoutbuf = (BYTE*)lpOutbuf;
		for (i = 0; i < dwInsize; i++)
		{
			BYTE inchar = *(((BYTE*)lpInbuf) + i);
			*(tmpoutbuf + i * 2) = ((inchar & 0xf) > 9) ? ((inchar & 0xf) - 10 + 'A') : ((inchar & 0xf) + '0');
			*(tmpoutbuf + i * 2 + 1) = (((inchar >> 4) & 0xf) > 9) ? (((inchar >> 4) & 0xf) - 10 + 'A') : (((inchar >> 4) & 0xf) + '0');
		}
		*lpOutsize = dwInsize * 2;

	}

	void __Text16To64(void* lpBuf, DWORD dwSize)
	{
		DWORD i;
		for (i = 0; i < dwSize; i++)
		{

			BYTE	dwMul = (BYTE)((i * 237) >> 1) % 4;
			BYTE	dwAdd = (BYTE)((i * 237) >> 2) % 4;
			BYTE* bSure = ((BYTE*)lpBuf) + i;
			*bSure -= '0';
			if (*bSure > 9)*bSure -= 'A' - '9' - 1;
			*bSure *= (dwMul + 1);
			*bSure += dwAdd;
			if (*bSure < 10) *bSure += '0';
			else if (*bSure < 36)*bSure += 'A' - 10;
			else if (*bSure < 62)*bSure += 'a' - 36;
			else if (*bSure == 62)*bSure = ':';
			else if (*bSure == 63)*bSure = '_';
		}
	}

	BOOL __EncryptData(void* lpInbuf, DWORD dwInsize, void* lpOutbuf, DWORD* lpOutsize)
	{
		void* tmpbuf = malloc(dwInsize * 2);
		DWORD outsize;
		if (tmpbuf == NULL)
			return FALSE;
		DWORD i, j;
		__Encrypt(lpInbuf, dwInsize, tmpbuf, &outsize);
		for (i = 0; i < outsize; i++)
		{
			for (j = 0; j < outsize; j++)
			{
				if (i != j)*(((BYTE*)tmpbuf) + j) += *(((BYTE*)tmpbuf) + i);
			}
		}
		__Bin2Text(tmpbuf, outsize, lpOutbuf, lpOutsize);
		__Text16To64(lpOutbuf, *lpOutsize);
		free(tmpbuf);
		return TRUE;

	}

#ifndef _WIN64
	BOOL GetCPUID(char* szCPUID)
	{
		unsigned long s1, s2;
		unsigned char vendor_id[] = "------------";
		char sel;
		sel = '1';
		char CPUID1[32];
		char CPUID2[32];
		memset(CPUID1, 0, sizeof(CPUID1));
		memset(CPUID2, 0, sizeof(CPUID2));
		__asm {
			xor eax, eax
			cpuid
			mov dword ptr vendor_id, ebx
			mov dword ptr vendor_id[+4], edx
			mov dword ptr vendor_id[+8], ecx
		}
		__asm {
			mov eax, 01h
			xor edx, edx
			cpuid
			mov s1, edx
			mov s2, eax
		}
		sprintf(CPUID1, "%08X%08X", s1, s2);
		__asm {
			mov eax, 03h
			xor ecx, ecx
			xor edx, edx
			cpuid
			mov s1, edx
			mov s2, ecx
		}
		sprintf(CPUID2, "%08X%08X", s1, s2);
		//	strcpy(szCPUID, (const char *)vendor_id);
		//	strcat(szCPUID, "{");
		strcat(szCPUID, CPUID1);
		strcat(szCPUID, CPUID2);
		//	strcat(szCPUID, "}");
		return TRUE;

	}
#endif // #ifndef _WIN64

	DWORD __GetClientUniqueCode(OUT LPSTR lpszCUC)
	{
		DWORD dwResult = 0;
		//	DWORD dwSysDiskNum = 0;
		//	PIDINFO lpID = NULL;
		TCHAR szPath[MAX_PATH] = { 0 };

		do
		{
			if (NULL == lpszCUC)
			{
				dwResult = FormatError(ERROR_INVALID_PARAMETER); break;
			}
			*lpszCUC = 0;
			//////////////////////////////////////////////////////////////////////////
	/*		DWORD dwBufBytes = max(sizeof(IDINFO),8192);
			lpID = (PIDINFO)malloc(dwBufBytes);
			if ( NULL == lpID )
			{
				dwResult = FormatError(ERROR_NOT_ENOUGH_MEMORY); break;
			}
			//////////////////////////////////////////////////////////////////////////
			::GetWindowsDirectory( szPath, MAX_PATH-1 );
			_tcsupr( szPath );
			TCHAR cLetter = szPath[0];
			sprintf( szPath, "\\\\.\\%C:", cLetter );
			dwResult = GetDiskNumber( szPath, &dwSysDiskNum, NULL, NULL, NULL );
			if ( dwResult )
			{
				FormatError(dwResult); break;
			}
			dwResult = GetDiskIdInfo(dwSysDiskNum, lpID);
			if ( dwResult )
			{
				FormatError(dwResult); break;
			}
			//////////////////////////////////////////////////////////////////////////
			CHAR szCode[64] = {0};
			RtlZeroMemory(szCode, sizeof(szCode));
			strcpy(szCode, lpID->sSerialNumber);
			szCode[32] = 0;
	*/		//////////////////////////////////////////////////////////////////////////
			CHAR szCode[64] = { 0 };
#ifdef _WIN64
			dwResult = FormatError(ERROR_NOT_SUPPORTED); break;
#else
			GetCPUID(szCode);
			szCode[32] = 0;
#endif
			//////////////////////////////////////////////////////////////////////////
			CHAR szData[MAX_PATH] = { 0 };
			RtlZeroMemory(szData, sizeof(szData));
			DWORD outsize = 0;
			__EncryptData(szCode, strlen(szCode), szData, &outsize);
			//////////////////////////////////////////////////////////////////////////
			RtlZeroMemory(szCode, sizeof(szCode));
			for (int i = 0; i < 32; i++)
			{
				szCode[i] = szData[i * outsize / 32];
				if (szCode[i] >= 'a' && szCode[i] <= 'z')szCode[i] -= 'a' - 'A';
			}
			szCode[16] = 0;
			strcpy(lpszCUC, szCode);
		} while (0);
		//	if ( lpID )	free(lpID);

		return dwResult;
	}

	std::string GetComputerID()
	{
		CHAR szCUC[32] = { 0 };
		__GetClientUniqueCode(szCUC);
		return std::string(szCUC);
	}

	DWORD GetCurrentProcessId() {
		return ::getpid();
	}
	HWND FindMainWindow(DWORD processId)
	{
		handle_data data;
		data.processId = processId;
		data.best_handle = 0;
		EnumWindows(EnumWindowsCallback, (LPARAM)&data);
		return data.best_handle;
	}
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
	{
		handle_data& data = *(handle_data*)lParam;
		HWND hwnd = ::GetWindow(handle, GW_OWNER);
		data.best_handle = handle;
		return 0;

		//unsigned long processId = 0;
		//::GetWindowThreadProcessId(handle, &processId);
		//if (data.processId != processId || !IsMainWindow(handle)) {
		//	return TRUE;
		//}
		//data.best_handle = handle;
		return FALSE;
	}

	std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::Utf8String& _proccname) {

		std::vector<PROCESSENTRY32W> infos;
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		PROCESSENTRY32W pe;
		pe.dwSize = sizeof(PROCESSENTRY32W);
		for (auto status = ::Process32FirstW(hSnapshot, &pe); status != FALSE; status = ::Process32NextW(hSnapshot, &pe)) {
			pe.dwSize = sizeof(PROCESSENTRY32W);
			Text::Utf8String item = pe.szExeFile;
			//�����������Ʋ�ѯ����
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
		DWORD result = ::GetModuleFileNameExW(hProcess, NULL, buf, sizeof(buf));
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
		//3���ж�ע������Ƿ��Ѿ�����
		WCHAR strDir[MAX_PATH]{ 0 };
		DWORD nLength = MAX_PATH;
		LSTATUS status = RegGetValueW(subKey, NULL, appName.utf16().c_str(), REG_SZ, NULL, strDir, &nLength);
		if (status != ERROR_SUCCESS) {
			if (bootstart == Text::Utf8String(strDir)) {
				bResult = true;
			}
		}
		::RegCloseKey(subKey);
		return bResult;
	}
	bool SetAutoBoot(const Text::Utf8String& filename, bool bStatus)
	{
		//�����Ҫ���ó������������Ѿ����������ͷ���true
		if (bStatus && GetAutoBootStatus(filename)) {
			return true;
		}
		if (!bStatus && !GetAutoBootStatus(filename)) {
			return true;
		}
		Text::Utf8String bootstart = filename.empty() ? Path::StartFileName() : filename;
		Text::Utf8String appName = Path::GetFileNameWithoutExtension(bootstart);
		bool bResult = false;
		HKEY subKey;
		if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\\", NULL, KEY_ALL_ACCESS, &subKey))
		{
			return bResult;
		}
		if (bStatus)
		{
			auto wStr = bootstart.utf16();
			if (ERROR_SUCCESS == ::RegSetValueExW(subKey, appName.utf16().c_str(), NULL, REG_SZ, (PBYTE)wStr.c_str(), wStr.size() * 2))
			{
				bResult = true;
			}
		}
		else
		{
			if (ERROR_SUCCESS == ::RegDeleteValueW(subKey, appName.utf16().c_str()))
			{
				bResult = true;
			}
		}
		::RegCloseKey(subKey);
		return bResult;
	}
	BOOL EnablePrivilege(HANDLE process)
	{
		// �õ����ƾ��
		HANDLE hToken = NULL;
		bool bResult = false;
		if (OpenProcessToken(process ? process : ::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken)) {
			// �õ���Ȩֵ
			LUID luid;
			if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
				// �������ƾ��Ȩ��
				TOKEN_PRIVILEGES tp = {};
				tp.PrivilegeCount = 1;
				tp.Privileges[0].Luid = luid;
				tp.Privileges[0].Attributes = 1 ? SE_PRIVILEGE_ENABLED : 0;
				if (AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL)) {
					bResult = true;
				}
			}
		}
		// �ر����ƾ��
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
					SHGetSpecialFolderPathW(0, buf, CSIDL_DESKTOPDIRECTORY, 0);//��ȡ��ǰ�û�����·��
					Text::Utf8String userDesktop(buf);
					if (!LnkName.empty()) {
						userDesktop += "\\" + LnkName + ".lnk";
					}
					else {
						userDesktop += "\\" + Path::GetFileNameWithoutExtension(pragmaFilename) + ".lnk";
					}
					//���ÿ�ݷ�ʽ��ַ
					File::Delete(userDesktop);//ɾ���ɵ�
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
		SHGetSpecialFolderPathW(0, buf, CSIDL_DESKTOPDIRECTORY, 0);//��ȡ��ǰ�û�����·��
		Text::Utf8String userDesktop(buf);
		if (!LnkName.empty()) {
			userDesktop += "\\" + LnkName + ".lnk";
		}
		else {
			userDesktop += "\\" + Path::GetFileNameWithoutExtension(pragmaFilename) + ".lnk";
		}
		//���ÿ�ݷ�ʽ��ַ
		File::Delete(userDesktop);//ɾ���ɵ�
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
		::RegDeleteValueW(subKey, L"UninstallString");
		::RegDeleteValueW(subKey, L"InstallLocation");
		// �ر�ע����
		RegCloseKey(subKey);
	}

	bool RegisterSoftware(const AppInfo& appInfo)
	{
		Text::Utf8String regKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
		regKeyPath.append("\\" + Path::GetFileNameWithoutExtension(appInfo.StartLocation));
		// ����ע�����
		HKEY hKey;

		SECURITY_DESCRIPTOR securityDesc;
		if (!InitializeSecurityDescriptor(&securityDesc, SECURITY_DESCRIPTOR_REVISION))
		{
			DWORD error = GetLastError();
			wprintf(L"�޷���ʼ����ȫ��������������룺%d\n", error);
			return false;
		}
		if (!SetSecurityDescriptorDacl(&securityDesc, TRUE, NULL, FALSE))
		{
			DWORD error = GetLastError();
			wprintf(L"�޷�����DACL��������룺%d\n", error);
			return false;
		}
		SECURITY_ATTRIBUTES attr;
		attr.nLength = sizeof(attr);
		attr.lpSecurityDescriptor = &securityDesc;
		attr.bInheritHandle = FALSE;

		LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, regKeyPath.utf16().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &attr, &hKey, NULL);
		if (result != ERROR_SUCCESS) {
			std::cerr << "Failed to create registry key." << std::endl;
			return false;
		}
		RegSetSoftware(hKey, L"DisplayName", appInfo.DisplayName);
		RegSetSoftware(hKey, L"DisplayVersion", appInfo.DisplayVersion);
		RegSetSoftware(hKey, L"Publisher", appInfo.DisplayAuthor);
		RegSetSoftware(hKey, L"UninstallString", appInfo.UninstallString);
		RegSetSoftware(hKey, L"InstallLocation", Path::GetDirectoryName(appInfo.StartLocation));//��װλ��

		if (appInfo.DesktopLnk) {
			WinTool::CreateDesktopLnk(appInfo.StartLocation, appInfo.DisplayName);
		}
		if (appInfo.AutoBoot) {
			WinTool::SetAutoBoot(appInfo.StartLocation, true);
		}
		else {
			WinTool::SetAutoBoot(appInfo.StartLocation, false);
		}
		// �ر�ע����
		RegCloseKey(hKey);
		return true;
	}
};