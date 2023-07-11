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
		/// ������������·�� C:\\Program Files\\xxx\xxx.exe
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
	BOOL EnablePrivilege(HANDLE process = NULL);
	//���������ݷ�ʽ
	bool CreateDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName = L"", const Text::Utf8String& cmdline = L"", const Text::Utf8String& iconFilename = L"");
	void DeleteDesktopLnk(const Text::Utf8String& pragmaFilename, const Text::Utf8String& LnkName);
	bool RegisterSoftware(const AppInfo& appInfo);
	void UnRegisterSoftware(const Text::Utf8String& appName_en);
	//���ó���������
	bool SetAutoBoot(const Text::Utf8String& filename = L"", bool enable = true);
	//��ȡ����������״̬
	bool GetAutoBootStatus(const Text::Utf8String& filename);
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
	HWND FindMainWindow(DWORD processId);
	BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
	//��ȡ������Ϣ
	std::vector<PROCESSENTRY32W> FindProcessInfo(const Text::Utf8String& _proccname);
	//���ݽ������ƴ򿪽���
	HANDLE OpenProcess(const Text::Utf8String& _proccname);
	//��ȡ����ID����
	std::vector<DWORD> FindProcessId(const Text::Utf8String& proccname);
	//��ȡ�����ļ�·��
	Text::Utf8String FindProcessFilename(DWORD processId);
	//�ر����н���
	int CloseProcess(const std::vector<DWORD>& processIds);
	//ʹ�ý���ID�رս���
	bool CloseProcess(DWORD processId);
	//��ȡ�����ǲ���64λ��
	bool Is64BitPorcess(DWORD processId);
	bool Is86BitPorcess(DWORD processId);
	//��ȡ��ǰ����ID
	DWORD GetCurrentProcessId();
	//��ȡϵͳ��Ϣ
	void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo);
	//��ȡϵͳλ��
	int GetSystemBits();
	//��ȡ�����Ψһʶ����
	std::string GetComputerID();

};
