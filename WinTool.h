#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
#include <winuser.h>
#include <TlHelp32.h>
#include <vector>
namespace WinTool {
	BOOL CALLBACK EnumNotepadChildWindowsProc(HWND hWnd, LPARAM lParam);
	inline BOOL IsMainWindow(HWND handle);
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
	inline HWND FindMainWindow(DWORD process_id);
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
	std::string wtoa(const std::wstring &wstr);
	//获取进程ID集合
	std::vector<DWORD> GetProcessId(const std::string& proccname);
	//关闭所有进程
	int CloseProcess(const std::vector<DWORD>& pids);
	//使用进程ID关闭进程
	bool CloseProcess(DWORD unProcessID);
	struct handle_data {
		unsigned long process_id;
		HWND best_handle;
	};
}
