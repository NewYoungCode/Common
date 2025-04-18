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
#include "base64.h"

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
		/// <summary>
		/// app名称
		/// </summary>
		Text::String DisplayName;
		/// <summary>
		/// 版本号
		/// </summary>
		Text::String DisplayVersion;
		/// <summary>
		/// 程序作者
		/// </summary>
		Text::String DisplayAuthor;
		/// <summary>
		/// 程序启动完整路径 C:\\Program Files\\xxx\xxx.exe //这是必须要传的
		/// </summary>
		Text::String StartLocation;
		/// <summary>
		/// 卸载执行的命令行
		/// </summary>
		Text::String UninstallString;
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
	extern bool CreateDesktopLnk(const Text::String& pragmaFilename, const Text::String& LnkName = L"", const Text::String& cmdline = L"", const Text::String& iconFilename = L"");
	//删除桌面快捷方式
	extern void DeleteDesktopLnk(const Text::String& pragmaFilename, const Text::String& LnkName);
	/// <summary>
	/// 删除注册表中某个项及其子项和值
	/// </summary>
	/// <param name="hKeyParent: ">HKEY_CURRENT_USER</param>
	/// <param name="subKey: ">"Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR"</param>
	extern void DeleteKeyRecursively(HKEY hKeyParent, const wchar_t* subKey);
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
	//获取进程是不是64位的
	extern bool Is64BitPorcess(DWORD processId);
	extern bool Is86BitPorcess(DWORD processId);
	//获取当前进程ID
	extern DWORD GetCurrentProcessId();
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
	extern double GetDiskFreeSize(const Text::String& path);
	//编码 加密
	extern void EnCode(const File::FileStream* fileData, File::FileStream* outData);
	//解码 解密
	extern void DeCode(const File::FileStream* fileData, File::FileStream* outData);
	/// <summary>
	/// 执行cmd并返回打印的字符
	/// </summary>
	/// <param name="cmdStr"></param>
	/// <returns></returns>
	extern Text::String ExecuteCMD(const Text::String& cmdStr);
	/// <summary>
	/// 获取主板序唯一标识
	/// </summary>
	/// <returns></returns>
	extern  Text::String GetBiosUUID();
	/// <summary>
	/// 获取CPU序列号
	/// </summary>
	/// <returns></returns>
	extern Text::String GetCPUSerialNumber();
	/// <summary>
	/// 获取硬盘序列号
	/// </summary>
	/// <returns></returns>
	extern  Text::String GetDiskSerialNumber();
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
	extern Text::String ShowFileDialog(HWND ownerWnd = NULL, Text::String defaultPath = "", Text::String title = "Select a File");
	/// <summary>
	/// 选择目录
	/// </summary>
	/// <param name="ownerWnd"></param>
	/// <param name="defaultPath"></param>
	/// <param name="title"></param>
	/// <returns></returns>
	extern Text::String ShowFolderDialog(HWND ownerWnd = NULL, Text::String defaultPath = "", Text::String title = "Select a directory");
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
};
