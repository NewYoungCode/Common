#pragma once
#include "Text.h"
//定义................................................
namespace Text {
	size_t String::length() const {
		auto* p = this->c_str();
		size_t pos = 0, count = 0;
		while (p[pos] && pos < this->size()) {
			if ((p[pos] & 0xc0) != 0x80) {
				count++;
			}
			pos++;
		}
		return count;
	}
	String::String() {}
	String::String(const std::string& str)noexcept {
		this->resize(str.size());
		::memcpy((void*)c_str(), str.c_str(), str.size());
	}
	String::String(const char* szbuf)noexcept {
		if (szbuf == NULL)return;
		size_t len = ::strlen(szbuf);
		this->resize(len);
		::memcpy((void*)c_str(), szbuf, len);
	}
	String::String(const wchar_t* szbuf)noexcept {
		if (szbuf == NULL)return;
		String::UnicodeToUTF8(szbuf, this);
	}
	String::String(const std::wstring& wstr)noexcept {
		String::UnicodeToUTF8(wstr, this);
	}

	//常用函数
	String String::Erase(const char& _char)const {
		String newStr(*this);
		String::Erase(&newStr, _char);
		return newStr;
	}
	std::vector<Text::String> String::Split(const String& ch_)const {
		std::vector<Text::String> arr;
		String::Split(*this, ch_, &arr);
		return arr;
	}
	String String::Replace(const String& oldText, const String& newText) const
	{
		String newStr = *this;
		String::Replace(&newStr, oldText, newText);
		return newStr;
	}
	String String::Tolower() const
	{
		String str(*this);
		String::Tolower(&str);
		return str;
	}
	String String::Toupper() const
	{
		String str(*this);
		String::Toupper(&str);
		return str;
	}
	void String::Append(const String& text)
	{
		__super::append(text);
	}
	size_t String::Find(const String& str, size_t Off)const
	{
		return __super::find(str, Off);
	}
	std::wstring String::unicode() const {
		std::wstring wstr;
		String::UTF8ToUnicode(*this, &wstr);
		return wstr;
	}
	std::string String::ansi() const {
		std::string str;
		String::UTF8ToANSI(*this, &str);
		return str;
	}

	void String::AnyToUnicode(const std::string& src_str, UINT codePage, std::wstring* out_wstr) {
		std::wstring& wstrCmd = *out_wstr;
		int bytes = ::MultiByteToWideChar(codePage, 0, src_str.c_str(), src_str.size(), NULL, 0);
		wstrCmd.resize(bytes);
		bytes = ::MultiByteToWideChar(codePage, 0, src_str.c_str(), src_str.size(), const_cast<wchar_t*>(wstrCmd.c_str()), wstrCmd.size());
	}
	void String::UnicodeToAny(const std::wstring& wstr, UINT codePage, std::string* out_str) {
		std::string& strCmd = *out_str;
		int bytes = ::WideCharToMultiByte(codePage, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		strCmd.resize(bytes);
		bytes = ::WideCharToMultiByte(codePage, 0, wstr.c_str(), wstr.size(), const_cast<char*>(strCmd.c_str()), strCmd.size(), NULL, NULL);
	}

	//以下是静态函数
	void String::ANSIToUniCode(const std::string& str, std::wstring* outStr)
	{
		AnyToUnicode(str, ::GetACP(), outStr);
	}
	void String::UnicodeToANSI(const std::wstring& wstr, std::string* outStr)
	{
		UnicodeToAny(wstr, ::GetACP(), outStr);
	}

	void String::GBKToUTF8(const std::string& str, std::string* outStr) {
		const int gbkCodePage = 936;
		std::wstring wstr;
		AnyToUnicode(str, gbkCodePage, &wstr);
		UnicodeToUTF8(wstr, outStr);
	}

	void String::UTF8ToGBK(const std::string& str, std::string* outStr) {
		const int gbkCodePage = 936;
		std::wstring wstr;
		UTF8ToUnicode(str, &wstr);
		UnicodeToAny(wstr, gbkCodePage, outStr);
	}

	void String::ANSIToUTF8(const std::string& str, std::string* outStr)
	{
		UINT codePage = ::GetACP();
		if (codePage == CP_UTF8) {
			*outStr = str;//如果本身就是utf8则不需要转换
			return;
		}
		std::wstring wstr;
		AnyToUnicode(str, codePage, &wstr);
		UnicodeToUTF8(wstr, outStr);
	}
	void String::UTF8ToANSI(const std::string& str, std::string* outStr) {
		UINT codePage = ::GetACP();
		if (codePage == CP_UTF8) {
			*outStr = str;//如果本身就是utf8则不需要转换
			return;
		}
		std::wstring wstr;
		UTF8ToUnicode(str, &wstr);
		UnicodeToAny(wstr, codePage, outStr);
	}
	void String::UnicodeToUTF8(const std::wstring& wstr, std::string* outStr)
	{
		UnicodeToAny(wstr, CP_UTF8, outStr);
	}
	void String::UTF8ToUnicode(const std::string& str, std::wstring* outStr) {
		AnyToUnicode(str, CP_UTF8, outStr);
	}


	void String::Tolower(std::string* str_in_out)
	{
		std::string& str = *str_in_out;
		for (size_t i = 0; i < str.size(); i++)
		{
			char& ch = (char&)str.c_str()[i];
			if (ch >= 65 && ch <= 90) {
				ch += 32;
			}
		}
	}
	void String::Toupper(std::string* str_in_out)
	{
		std::string& str = *str_in_out;
		for (size_t i = 0; i < str.size(); i++)
		{
			char& ch = (char&)str.c_str()[i];
			if (ch >= 97 && ch <= 122) {
				ch -= 32;
			}
		}
	}
	void String::Erase(std::string* str_in_out, const char& _char) {
		const String& self = *str_in_out;
		char* bufStr = new char[self.size() + 1]{ 0 };
		size_t pos = 0;
		for (auto& it : self) {
			if (_char == it)continue;
			bufStr[pos] = it;
			pos++;
		}
		*str_in_out = bufStr;
		delete[] bufStr;
	}
	void String::Replace(std::string* str_in_out, const std::string& oldText, const std::string& newText)
	{
		std::string& newStr = *str_in_out;
		size_t pos;
		pos = newStr.find(oldText);
		while (pos != std::string::npos)
		{
			newStr.replace(pos, oldText.size(), newText);
			pos = newStr.find(oldText);
		}
	}
	void String::Split(const std::string& str_in, const std::string& ch_, std::vector<String>* strs_out) {

		std::vector<String>& arr = *strs_out;
		arr.clear();
		if (str_in.empty()) return;

		std::string buf = str_in;
		size_t pos = buf.find(ch_);
		if (pos == std::string::npos) {
			arr.push_back(buf);
			return;
		}
		while (pos != std::string::npos) {
			auto item = buf.substr(0, pos);
			if (!item.empty()) {
				arr.push_back(item);
			}
			buf = buf.erase(0, pos + ch_.size());
			pos = buf.find(ch_);
			if (pos == std::string::npos) {
				if (!buf.empty()) {
					arr.push_back(buf);
				}
			}
		}
	}

	String ToString(double number, int keepBitSize) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(keepBitSize) << number;
		return oss.str();
	}
};