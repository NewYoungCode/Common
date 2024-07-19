#pragma once
#include <Windows.h>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
namespace Text {
	/// <summary>
	/// utf8字符串
	/// </summary>
	class String :public std::string {
	public:
		String();
		virtual ~String();
		String(const std::string& str)noexcept;
		String(const char* szbuf)noexcept;
		String(const wchar_t* szbuf)noexcept;
		String(const std::wstring& wstr)noexcept;
		size_t length() const;//the utf8 Length
		std::wstring unicode() const;
		std::string ansi() const;
		std::vector<std::string> split(const String& ch_)const;
		void erase(char _ch);
		void erase(size_t pos, size_t count);
		String replace(char oldChar, char newChar);
		String replace(const String& oldText, const String& newText, bool allReplace = true)const;
		String toLower()const;
		String toUpper()const;
		bool operator==(const wchar_t* szbuf)const;
		bool operator==(const std::wstring& wStr)const;
	};

	//base convert
	extern void AnyToUnicode(const std::string& src_str, UINT codePage, std::wstring* out_wstr);
	extern void UnicodeToAny(const std::wstring& unicode_wstr, UINT codePage, std::string* out_str);
	//
	extern void GBKToUTF8(const std::string& str, std::string* outStr);
	extern void UTF8ToGBK(const std::string& str, std::string* outStr);
	extern void ANSIToUniCode(const std::string& str, std::wstring* outStr);
	extern void ANSIToUTF8(const std::string& str, std::string* outStr);
	extern void UnicodeToANSI(const std::wstring& wstr, std::string* outStr);
	extern void UnicodeToUTF8(const std::wstring& wstr, std::string* outStr);
	extern void UTF8ToANSI(const std::string& str, std::string* outStr);
	extern void UTF8ToUnicode(const std::string& str, std::wstring* outStr);
	//
	extern void Tolower(std::string* str_in_out);
	extern void Toupper(std::string* str_in_out);
	extern void Erase(std::string* str_in_out, const char& ch);
	extern void Replace(std::string* str_in_out, char oldChar, char newChar);
	extern void Replace(std::string* str_in_out, const std::string& oldText, const std::string& newText, bool replaceAll = true);
	extern void Split(const std::string& str_in, const std::string& ch, std::vector<std::string>* strs_out);
	//
	extern String ToString(double number, int keepBitSize);
#define utf8(text) String(L##text)
};
