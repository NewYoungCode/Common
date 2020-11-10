#include "WinTool.h"

namespace WinTool {
	  BOOL CALLBACK EnumNotepadChildWindowsProc(HWND hWnd, LPARAM lParam)
	{
		char szTitle[100] = { 0 };
		::GetWindowTextA(hWnd, szTitle, 100);
		/*SetWindowTextA(hWnd, "就尼玛离谱");
		SendMessage(hWnd, WM_PAINT, NULL, NULL);*/
		//SendMessage(hWnd, BM_CLICK, NULL, NULL);
		long lStyle = GetWindowLong(hWnd, GWL_STYLE);
		if (lStyle & ES_MULTILINE)
		{
			long lineCount = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0);
			for (int i = 0; i < lineCount; i++)
			{
				//long chCount = SendMessage(hWnd, EM_LINELENGTH, (WPARAM)i, 0);
				char szContent[200] = { 0 };
				szContent[0] = 200; //此处注意下，如果不设置EM_GETLINE无法获取内容
				long ret = SendMessage(hWnd, EM_GETLINE, (WPARAM)i, (LPARAM)(LPCSTR)szContent);
				if (ret > 0)
				{
					szContent[ret] = '\0';
					std::cout << "line: " << i << ", Content: " << szContent << std::endl;
				}
			}
		}
		else
		{
			std::string title(szTitle);
			if (!title.empty())
				std::cout << title << std::endl;
		}
		return true;
	}
	  BOOL IsMainWindow(HWND handle)
	{
		return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
	}
	  HWND FindMainWindow(DWORD process_id)
	{
		handle_data data;
		data.process_id = process_id;
		data.best_handle = 0;
		EnumWindows(EnumWindowsCallback, (LPARAM)&data);
		return data.best_handle;
	}
	  BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
	{
		handle_data& data = *(handle_data*)lParam;
		unsigned long process_id = 0;
		GetWindowThreadProcessId(handle, &process_id);
		if (data.process_id != process_id || !IsMainWindow(handle)) {
			return TRUE;
		}
		data.best_handle = handle;
		return FALSE;
	}
	   std::string wtoa(const std::wstring &wstr)
	{
		int bytes = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		std::string strCmd;
		strCmd.resize(bytes);
		bytes = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), const_cast<char*>(strCmd.data()), strCmd.size(), NULL, NULL);
		return strCmd;
	}
	  std::vector<DWORD>  GetProcessId(const std::string& _proccname)
	{
		std::vector<DWORD> pids;
		PROCESSENTRY32 pe;
		DWORD id = 0;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe))
			return pids;
		while (true)
		{
			pe.dwSize = sizeof(PROCESSENTRY32);
			if (Process32Next(hSnapshot, &pe) == FALSE)
				break;
			std::string item = pe.szExeFile;
			if (item == _proccname) {
				id = pe.th32ProcessID;
				pids.push_back(id);
			}
		}
		CloseHandle(hSnapshot);
		return pids;
	}
	  int CloseProcess(const std::vector<DWORD>& pids) {
		size_t count = 0;
		for (auto item : pids) {
			count += CloseProcess(item);
		}
		return count;
	}
	  bool CloseProcess(DWORD unProcessID)
	{
		HANDLE bExitCode = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE
			| PROCESS_ALL_ACCESS, FALSE, unProcessID);
		if (bExitCode)
		{
			BOOL bFlag = TerminateProcess(bExitCode, 0);
			CloseHandle(bExitCode);
			return true;
		}
		return false;
	}
};