#pragma once
#include "Common.h"
#include <TlHelp32.h>
#include <ShlObj.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include <psapi.h>
#include <process.h>

namespace WinTool {
	typedef struct {
		unsigned long processId;
		HWND best_handle;
	}handle_data;

	struct AppInfo {
		/// <summary>
		/// app名称
		/// </summary>
		Text::Utf8String DisplayName;
		/// <summary>
		/// 版本号
		/// </summary>
		Text::Utf8String DisplayVersion;
		/// <summary>
		/// 程序作者
		/// </summary>
		Text::Utf8String DisplayAuthor;
		/// <summary>
		/// 程序启动完整路径 C:\\Program Files\\xxx\xxx.exe
		/// </summary>
		Text::Utf8String StartLocation;
		/// <summary>
		/// 卸载执行的命令行
		/// </summary>
		Text::Utf8String UninstallString;
		/// <summary>
		/// 是否创建桌面快捷方式
		/// </summary>
		bool DesktopLnk = true;
		/// <summary>
		/// 是否开机启动
		/// </summary>
		bool AutoBoot = false;
	};
	//给进程提权
	BOOL EnablePrivilege(HANDLE process = NULL);
	//创建桌面快捷方式
	bool CreateDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName = L"", const Text::Utf8String& cmdline = L"", const Text::Utf8String& iconFilename = L"");
	void DeleteDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName);
	bool RegisterSoftware(const AppInfo& appInfo);
	void UnRegisterSoftware(const Text::Utf8String& appName_en);
	//设置程序自启动
	bool SetAutoBoot(const Text::Utf8String& filename = L"", bool enable = true);
	//获取程序自启动状态
	bool GetAutoBootStatus(const Text::Utf8String& filename);
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
	HWND FindMainWindow(DWORD processId);
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
	//获取进程信息
	std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::Utf8String& _proccname);
	//根据进程名称打开进程
	HANDLE OpenProcess(const Text::Utf8String& _proccname);
	//获取进程ID集合
	std::vector<DWORD> FindProcessId(const Text::Utf8String& proccname);
	//获取进程文件路径
	Text::Utf8String FindProcessFilename(DWORD processId);
	//关闭所有进程
	int CloseProcess(const std::vector<DWORD>& processIds);
	//使用进程ID关闭进程
	bool CloseProcess(DWORD processId);
	//获取进程是不是64位的
	bool Is64BitPorcess(DWORD processId);
	bool Is86BitPorcess(DWORD processId);
	//获取当前进程ID
	DWORD GetCurrentProcessId();
	//获取系统信息
	void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);
	//获取系统位数
	int GetSystemBits();
	//获取计算机唯一识别码
	std::string GetComputerID();

};
