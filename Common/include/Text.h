#pragma once
#include <Windows.h>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
namespace Text {
	//你最好使用它来存储utf8字符串
	class String :public std::string {
	public:
		//the utf8 length
		size_t length() const;
		String();
		String(const std::string& str)noexcept;
		String(const char* szbuf)noexcept;
		String(const wchar_t* szbuf)noexcept;
		String(const std::wstring& wstr)noexcept;
		std::wstring unicode() const;
		std::string ansi() const;
		String Erase(const char& _char)const;
		std::vector<Text::String> Split(const String& ch_)const;
		String Replace(const String& oldText, const String& newText)const;
		String Tolower()const;
		String Toupper()const;
		void Append(const String& text);
		size_t Find(const String& text, size_t Off = 0U)const;
	public:
		//base convert
		static void AnyToUnicode(const std::string& src_str, UINT codePage, std::wstring* out_wstr);
		static void UnicodeToAny(const std::wstring& unicode_wstr, UINT codePage, std::string* out_str);
		//
		static void GBKToUTF8(const std::string& str, std::string* outStr);
		static void UTF8ToGBK(const std::string& str, std::string* outStr);
		static void ANSIToUniCode(const std::string& str, std::wstring* outStr);
		static void ANSIToUTF8(const std::string& str, std::string* outStr);
		static void UnicodeToANSI(const std::wstring& wstr, std::string* outStr);
		static void UnicodeToUTF8(const std::wstring& wstr, std::string* outStr);
		static void UTF8ToANSI(const std::string& str, std::string* outStr);
		static void UTF8ToUnicode(const std::string& str, std::wstring* outStr);
		//
		static void Tolower(std::string* str_in_out);
		static void Toupper(std::string* str_in_out);
		static void Erase(std::string* str_in_out, const char& ch);
		static void Replace(std::string* str_in_out, const std::string& oldText, const std::string& newText);
		static void Split(const std::string& str_in, const std::string& ch, std::vector<String>* strs_out);
	};
#define utf8(text) EString(L##text)
	/// <summary>
	/// 小数转字符串
	/// </summary>
	/// <param name="number">数值</param>
	/// <param name="keepBitSize">保留位数</param>
	/// <returns></returns>
	extern String ToString(double number, int keepBitSize);
};
