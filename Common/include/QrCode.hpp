#pragma once
#include "Common.h"
namespace QrCode {
	inline bool BuildQRcode(const char* str, const wchar_t* fileName) {
		typedef bool (*BuildQrCodeFunc)(const char*, const wchar_t*);
		HMODULE hm = ::LoadLibraryW(L"QrCode.dll");
		if (hm) {
			BuildQrCodeFunc func = (BuildQrCodeFunc)::GetProcAddress(hm, "BuildQRcode");
			bool b = func(str, fileName);
			FreeLibrary(hm);
			return b;
		}
		else {
			::MessageBoxW(0, L"QrCode.dll don't exist !", L"", 0);
		}
		return false;
	}
};