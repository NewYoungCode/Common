#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <ShlObj.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include <psapi.h>
#include <process.h>
#include <map>

#include "Text.h"
#include "FileSystem.h"
#include "Util.h"
#include "Time.hpp"
#include "base64.h"

#ifdef GetUserName
#undef GetUserName
inline void GetUserName(LPSTR lpBuffer, LPDWORD pcbBuffer) {
	::GetUserNameA(lpBuffer, pcbBuffer);
}
inline void GetUserName(LPWSTR lpBuffer, LPDWORD pcbBuffer) {
	::GetUserNameW(lpBuffer, pcbBuffer);
}
#endif

#ifdef GetComputerName
#undef GetComputerName
inline void GetComputerName(LPSTR lpBuffer, LPDWORD pcbBuffer) {
	::GetComputerNameA(lpBuffer, pcbBuffer);
}
inline void GetComputerName(LPWSTR lpBuffer, LPDWORD pcbBuffer) {
	::GetComputerNameW(lpBuffer, pcbBuffer);
}
#endif

namespace WinTool {
	/// <summary>
	/// 路由信息
	/// </summary>
	struct RouterInfo {
		Text::String IP;
		Text::String MAC;
	};
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
		/// app名称
		Text::String DisplayName;
		/// 版本号
		Text::String DisplayVersion;
		//显示图标
		Text::String DisplayIcon;
		// 发布者
		Text::String Publisher;
		// 程序启动完整路径 C:\\Program Files\\xxx\xxx.exe //这是必须要传的
		Text::String StartLocation;
		// 卸载执行的命令行
		Text::String UninstallString;
		// 产品帮助链接
		Text::String HelpLink;
		//帮助说明[这是一款桌面工具]
		Text::String Comments;
		//产品官网
		Text::String URLInfoAbout;
		//是否禁用控制面板修改按钮
		bool NoModify = false;
		//是否禁用控制面板修复按钮
		bool NoRepair = false;
		// 是否创建桌面快捷方式
		bool DesktopLink = true;
		// 是否开机启动
		bool AutoBoot = false;
	};
	//给进程提权
	extern BOOL EnablePrivilege(HANDLE process = NULL);
	//创捷快捷方式
	extern bool CreateLink(const Text::String& pragmaFilename, const Text::String& outDir, const Text::String& LnkName = L"", const Text::String& cmdline = L"", const Text::String& iconFilename = L"");
	//删除快捷方式
	extern void DeleteLink(const Text::String& linkDir, const Text::String& pragmaFilename, const Text::String& LnkName = "");
	/// <summary>
	/// 删除注册表中某个项及其子项和值
	/// </summary>
	/// <param name="hKeyParent: ">HKEY_CURRENT_USER</param>
	/// <param name="subKey: ">"Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR"</param>
	extern void DeleteKeyRecursively(HKEY hKeyParent, const wchar_t* subKey);
	//获取软件版本信息
	extern Text::String GetSoftwareValue(const Text::String& appName_en, const Text::String& key);
	//根据软件名称修改一些信息
	extern bool RegSetSoftwareValue(const Text::String& appName_en, const Text::String& key, const Text::String& value);
	//注册软件到电脑
	extern bool RegisterSoftware(const AppInfo& appInfo);
	//从电脑上注销软件
	extern 	void UnRegisterSoftware(const Text::String& appName_en);
	//给软件注册许可
	extern bool RegisterLicenser(const Text::String& exeFilename, const Text::String& softwareData);
	//获取软件许可证书
	extern Text::String FindLicenser(const Text::String& exeFilename);
	//设置程序自启动
	extern bool SetAutoBoot(const Text::String& filename = L"", bool enable = true);
	//获取程序自启动状态
	extern bool GetAutoBootStatus(const Text::String& filename);
	//寻找进程中的窗口
	extern HWND FindMainWindow(DWORD processId);
	//获取进程信息
	extern std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::String& _proccname);
	//根据进程名称打开进程
	extern HANDLE OpenProcess(const Text::String& _proccname);
	//获取进程ID集合
	extern std::vector<DWORD> FindProcessId(const Text::String& proccname);
	//获取进程文件路径
	extern Text::String FindProcessFilename(DWORD processId);
	//关闭所有进程
	extern int CloseProcess(const std::vector<DWORD>& processIds);
	//使用进程ID关闭进程
	extern bool CloseProcess(DWORD processId);
	//使用句柄关闭进程
	extern bool CloseProcess(HANDLE hProcess, UINT exitCode = 0);
	//获取进程是不是64位的
	extern bool Is64BitPorcess(DWORD processId);
	//获取进程是不是32位的
	extern bool Is86BitPorcess(DWORD processId);
	//获取当前进程ID
	extern DWORD GetCurrentProcessId();
	//获取系统位数
	extern int GetSystemBits();
	//获取计算机唯一识别码
	extern Text::String GetComputerID();
	//用户当前用户名称
	extern Text::String GetUserName();
	//获取计算机名称
	extern Text::String GetComputerName();
	//获取网卡相关
	extern int GetAdptersInfo(std::vector<MyAdpterInfo>& adpterInfo);
	/// <summary>
	/// 获取磁盘可以用空间单位:GB
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	extern double GetDiskFreeSize(const Text::String& path);
	/// <summary>
	/// 直接执行可执行文件并获取返回内容
	/// </summary>
	/// <param name="cmdStr"></param>
	/// <returns></returns>
	extern Text::String ExecuteCMD(const Text::String& cmdStr, std::function<void(const Text::String&)> callback = NULL, HANDLE* outHandle = NULL);
	/// <summary>
	/// 获取首选网卡的mac地址
	/// </summary>
	/// <returns></returns>
	extern  Text::String GetMacAddress();
	/// <summary>
	/// 获取操作系统的版本号
	/// </summary>
	/// <returns></returns>
	extern Text::String GetWinVersion();
	/// <summary>
	/// 选择文件
	/// </summary>
	/// <param name="ownerWnd"></param>
	/// <param name="defaultPath"></param>
	/// <param name="title"></param>
	/// <returns></returns>
	extern Text::String ShowFileDialog(HWND ownerWnd = NULL, const Text::String& defaultPath = "", const Text::String& title = "Select a File", const Text::String& filter = "All Files\0*.*\0");
	/// <summary>
	/// 选择目录
	/// </summary>
	/// <param name="ownerWnd"></param>
	/// <param name="defaultPath"></param>
	/// <param name="title"></param>
	/// <returns></returns>
	extern Text::String ShowFolderDialog(HWND ownerWnd = NULL, const Text::String& defaultPath = "", const Text::String& title = "Select a directory");
	/// <summary>
	/// 获取路由信息
	/// </summary>
	/// <returns></returns>
	extern RouterInfo GetRouterInfo();
	/// <summary>
	/// 获取电脑的com端口名称
	/// </summary>
	/// <returns></returns>
	extern std::vector<Text::String> GetComPorts();
	/// <summary>
	/// 安装带有.inf文件的驱动
	/// </summary>
	/// <param name="infPath">.inf文件路径</param>
	/// <param name="needReboot">是否需要重启</param>
	/// <returns></returns>
	extern bool InstallDriver(const Text::String& infPath, bool* needReboot = NULL);
	/// <summary>
	/// 获取已安装的应用
	/// </summary>
	/// <returns>返回软件名称,安装路径</returns>
	extern std::map<Text::String, Text::String> GetApps();
	/// <summary>
	/// 检测程序是否被调试
	/// </summary>
	/// <returns></returns>
	extern bool CheckDebug();
	/// <summary>
	/// 检查程序是否正在运行 使用文件独占方式
	/// </summary>
	/// <param name="productName"></param>
	/// <returns></returns>
	extern bool IsRunning(const Text::String& productName = "", bool lock = true);
};
