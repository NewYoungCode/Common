#pragma once
#include "Text.h"
//定义................................................
namespace Text {
	size_t Utf8String::length() const {
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
	Utf8String::Utf8String() {}
	Utf8String::Utf8String(const std::string& str)noexcept {
		this->resize(str.size());
		::memcpy((void*)c_str(), str.c_str(), str.size());
	}
	Utf8String::Utf8String(const char* szbuf)noexcept {
		if (szbuf == NULL)return;
		size_t len = ::strlen(szbuf);
		this->resize(len);
		::memcpy((void*)c_str(), szbuf, len);
	}
	Utf8String::Utf8String(const wchar_t* szbuf)noexcept {
		if (szbuf == NULL)return;
		Utf8String::UnicodeToUTF8(szbuf, this);
	}
	Utf8String::Utf8String(const std::wstring& wstr)noexcept {
		Utf8String::UnicodeToUTF8(wstr, this);
	}

	//常用函数
	Utf8String Utf8String::Erase(const char& _char)const {
		Utf8String newStr(*this);
		Utf8String::Erase(&newStr, _char);
		return newStr;
	}
	std::vector<Text::Utf8String> Utf8String::Split(const Utf8String& ch_)const {
		std::vector<Text::Utf8String> arr;
		Utf8String::Split(*this, ch_, &arr);
		return arr;
	}
	Utf8String Utf8String::Replace(const Utf8String& oldText, const Utf8String& newText) const
	{
		Utf8String newStr = *this;
		Utf8String::Replace(&newStr, oldText, newText);
		return newStr;
	}
	Utf8String Utf8String::Tolower() const
	{
		Utf8String str(*this);
		Utf8String::Tolower(&str);
		return str;
	}
	Utf8String Utf8String::Toupper() const
	{
		Utf8String str(*this);
		Utf8String::Toupper(&str);
		return str;
	}
	void Utf8String::append(const Utf8String& text)
	{
		__super::append(text);
	}
	std::wstring Utf8String::utf16() const {
		std::wstring wstr;
		Utf8String::UTF8ToUnicode(*this, &wstr);
		return wstr;
	}
	std::string Utf8String::ansi() const {
		std::string str;
		Utf8String::UTF8ToANSI(*this, &str);
		return str;
	}

	void Utf8String::AnyToUnicode(const std::string& src_str, UINT codePage, std::wstring* out_wstr) {
		std::wstring& wstrCmd = *out_wstr;
		int bytes = ::MultiByteToWideChar(codePage, 0, src_str.c_str(), src_str.size(), NULL, 0);
		wstrCmd.resize(bytes);
		bytes = ::MultiByteToWideChar(codePage, 0, src_str.c_str(), src_str.size(), const_cast<wchar_t*>(wstrCmd.c_str()), wstrCmd.size());
	}
	void Utf8String::UnicodeToAny(const std::wstring& wstr, UINT codePage, std::string* out_str) {
		std::string& strCmd = *out_str;
		int bytes = ::WideCharToMultiByte(codePage, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		strCmd.resize(bytes);
		bytes = ::WideCharToMultiByte(codePage, 0, wstr.c_str(), wstr.size(), const_cast<char*>(strCmd.c_str()), strCmd.size(), NULL, NULL);
	}

	//以下是静态函数
	void Utf8String::ANSIToUniCode(const std::string& str, std::wstring* outStr)
	{
		AnyToUnicode(str, ::GetACP(), outStr);
	}
	void Utf8String::UnicodeToANSI(const std::wstring& wstr, std::string* outStr)
	{
		UnicodeToAny(wstr, ::GetACP(), outStr);
	}

	void Utf8String::GBKToUTF8(const std::string& str, std::string* outStr) {
		const int gbkCodePage = 936;
		std::wstring wstr;
		AnyToUnicode(str, gbkCodePage, &wstr);
		UnicodeToUTF8(wstr, outStr);
	}

	void Utf8String::UTF8ToGBK(const std::string& str, std::string* outStr) {
		const int gbkCodePage = 936;
		std::wstring wstr;
		UTF8ToUnicode(str, &wstr);
		UnicodeToAny(wstr, gbkCodePage, outStr);
	}

	void Utf8String::ANSIToUTF8(const std::string& str, std::string* outStr)
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
	void Utf8String::UTF8ToANSI(const std::string& str, std::string* outStr) {
		UINT codePage = ::GetACP();
		if (codePage == CP_UTF8) {
			*outStr = str;//如果本身就是utf8则不需要转换
			return;
		}
		std::wstring wstr;
		UTF8ToUnicode(str, &wstr);
		UnicodeToAny(wstr, codePage, outStr);
	}
	void Utf8String::UnicodeToUTF8(const std::wstring& wstr, std::string* outStr)
	{
		UnicodeToAny(wstr, CP_UTF8, outStr);
	}
	void Utf8String::UTF8ToUnicode(const std::string& str, std::wstring* outStr) {
		AnyToUnicode(str, CP_UTF8, outStr);
	}


	void Utf8String::Tolower(std::string* str_in_out)
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
	void Utf8String::Toupper(std::string* str_in_out)
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
	void Utf8String::Erase(std::string* str_in_out, const char& _char) {
		const Utf8String& self = *str_in_out;
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
	void Utf8String::Replace(std::string* str_in_out, const std::string& oldText, const std::string& newText)
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
	void Utf8String::Split(const std::string& str_in, const std::string& ch_, std::vector<Utf8String>* strs_out) {

		std::vector<Utf8String>& arr = *strs_out;
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

	Utf8String ToString(double number, int keepBitSize) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(keepBitSize) << number;
		return oss.str();
	}
};