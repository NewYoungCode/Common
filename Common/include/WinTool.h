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
		/// app����
		/// </summary>
		Text::Utf8String DisplayName;
		/// <summary>
		/// �汾��
		/// </summary>
		Text::Utf8String DisplayVersion;
		/// <summary>
		/// ��������
		/// </summary>
		Text::Utf8String DisplayAuthor;
		/// <summary>
		/// ������������·�� C:\\Program Files\\xxx\xxx.exe //���Ǳ���Ҫ����
		/// </summary>
		Text::Utf8String StartLocation;
		/// <summary>
		/// ж��ִ�е�������
		/// </summary>
		Text::Utf8String UninstallString;
		/// <summary>
		/// �Ƿ񴴽������ݷ�ʽ
		/// </summary>
		bool DesktopLnk = true;
		/// <summary>
		/// �Ƿ񿪻�����
		/// </summary>
		bool AutoBoot = false;
	};
	//��������Ȩ
	extern BOOL EnablePrivilege(HANDLE process = NULL);
	//���������ݷ�ʽ
	extern bool CreateDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName = L"", const Text::Utf8String& cmdline = L"", const Text::Utf8String& iconFilename = L"");
	extern void DeleteDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName);
	extern bool RegisterSoftware(const AppInfo& appInfo);
	extern 	void UnRegisterSoftware(const Text::Utf8String& appName_en);
	//���ó���������
	extern bool SetAutoBoot(const Text::Utf8String& filename = L"", bool enable = true);
	//��ȡ����������״̬
	extern bool GetAutoBootStatus(const Text::Utf8String& filename);
	extern HWND FindMainWindow(DWORD processId);
	//��ȡ������Ϣ
	extern std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::Utf8String& _proccname);
	//���ݽ������ƴ򿪽���
	extern HANDLE OpenProcess(const Text::Utf8String& _proccname);
	//��ȡ����ID����
	extern std::vector<DWORD> FindProcessId(const Text::Utf8String& proccname);
	//��ȡ�����ļ�·��
	extern Text::Utf8String FindProcessFilename(DWORD processId);
	//�ر����н���
	extern int CloseProcess(const std::vector<DWORD>& processIds);
	//ʹ�ý���ID�رս���
	extern bool CloseProcess(DWORD processId);
	//��ȡ�����ǲ���64λ��
	extern bool Is64BitPorcess(DWORD processId);
	extern bool Is86BitPorcess(DWORD processId);
	//��ȡ��ǰ����ID
	extern DWORD GetCurrentProcessId();
	//��ȡϵͳ��Ϣ
	extern void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);
	//��ȡϵͳλ��
	extern int GetSystemBits();
	//��ȡ�����Ψһʶ����
	extern std::string GetComputerID();
	//��ȡ�������
	extern int GetAdptersInfo(std::vector<MyAdpterInfo>& adpterInfo);
	/// <summary>
	/// ��ȡ���̿����ÿռ䵥λ:GB
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	extern double GetDiskFreeSize(const Text::Utf8String& path);
	//���� ����
	extern void EnCode(const File::FileStream* fileData, File::FileStream* outData);
	//���� ����
	extern void DeCode(const File::FileStream* fileData, File::FileStream* outData);
};
