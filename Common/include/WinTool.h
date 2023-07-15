#pragma once
#include "Common.h"
#include "HttpUtility.h"
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
	typedef struct _MyAdpterInfo
	{
		std::vector<std::string> Ip;
		std::string MacAddress;
		std::string Description;
		std::string Name;
		UINT Type;
	}MyAdpterInfo;
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
		/// 程序启动完整路径 C:\\Program Files\\xxx\xxx.exe //这是必须要传的
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
	extern BOOL EnablePrivilege(HANDLE process = NULL);
	//创建桌面快捷方式
	extern bool CreateDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName = L"", const Text::Utf8String& cmdline = L"", const Text::Utf8String& iconFilename = L"");
	extern void DeleteDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName);
	extern bool RegisterSoftware(const AppInfo& appInfo);
	extern 	void UnRegisterSoftware(const Text::Utf8String& appName_en);
	//设置程序自启动
	extern bool SetAutoBoot(const Text::Utf8String& filename = L"", bool enable = true);
	//获取程序自启动状态
	extern bool GetAutoBootStatus(const Text::Utf8String& filename);
	extern HWND FindMainWindow(DWORD processId);
	//获取进程信息
	extern std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::Utf8String& _proccname);
	//根据进程名称打开进程
	extern HANDLE OpenProcess(const Text::Utf8String& _proccname);
	//获取进程ID集合
	extern std::vector<DWORD> FindProcessId(const Text::Utf8String& proccname);
	//获取进程文件路径
	extern Text::Utf8String FindProcessFilename(DWORD processId);
	//关闭所有进程
	extern int CloseProcess(const std::vector<DWORD>& processIds);
	//使用进程ID关闭进程
	extern bool CloseProcess(DWORD processId);
	//获取进程是不是64位的
	extern bool Is64BitPorcess(DWORD processId);
	extern bool Is86BitPorcess(DWORD processId);
	//获取当前进程ID
	extern DWORD GetCurrentProcessId();
	//获取系统信息
	extern void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);
	//获取系统位数
	extern int GetSystemBits();
	//获取计算机唯一识别码
	extern std::string GetComputerID();
	//获取网卡相关
	extern int GetAdptersInfo(std::vector<MyAdpterInfo>& adpterInfo);
	/// <summary>
	/// 获取磁盘可以用空间单位:GB
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	extern double GetDiskFreeSize(const Text::Utf8String& path);
	//编码 加密
	extern void EnCode(const File::FileStream* fileData, File::FileStream* outData);
	//解码 解密
	extern void DeCode(const File::FileStream* fileData, File::FileStream* outData);
};
