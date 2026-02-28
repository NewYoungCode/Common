#include "Text.h"
#include <Windows.h>
#include <list>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Text {
	//-----------------------------------------------Copy Start-----------------------------------------------
	size_t String::utf8Length() const {
		const char* p = this->c_str();
		const char* end = p + this->size();
		size_t count = 0;
		// 优化：避免重复调用 size()，使用指针遍历
		while (p < end) {
			// UTF-8 字符的首字节不是 10xxxxxx
			if ((*p & 0xc0) != 0x80) {
				++count;
			}
			++p;
		}
		return count;
	}
	String::String() {}
	String::~String() {}
	String::String(const String& _right)noexcept :std::string(_right) {}
	String::String(String&& _right) noexcept :std::string(std::move(_right)) {}
	String& String::operator=(const String& _right) noexcept
	{
		(std::string&)*this = _right;
		return *this;
	}
	String& String::operator=(String&& _right) noexcept
	{
		std::string::operator=(std::move(_right));
		return *this;
	}
	String::String(const std::string& str)noexcept :std::string(str) {}
	String::String(const char* szbuf)noexcept :std::string(szbuf) {}
	String::String(const char* pStr, size_t count) noexcept :std::string(pStr, count) {}
	String::String(const wchar_t* szbuf)noexcept {
		if (szbuf == NULL)return;
		UnicodeToUTF8(szbuf, this);
	}
	String::String(const std::wstring& wstr)noexcept {
		UnicodeToUTF8(wstr, this);
	}
	void String::erase(char _ch)
	{
		Erase(this, _ch);
	}
	void String::erase(size_t pos, size_t count)
	{
		__super::erase(pos, count);
	}
	String String::replace(char oldChar, char newChar) const
	{
		String newStr = *this;
		Replace(&newStr, oldChar, newChar);
		return newStr;
	}
	String String::replace(const String& oldText, const String& newText, bool allReplace) const
	{
		String newStr = *this;
		Replace(&newStr, oldText, newText, allReplace);
		return newStr;
	}
	String String::toLower() const
	{
		String str(*this);
		Tolower(&str);
		return str;
	}
	String String::toUpper() const
	{
		String str(*this);
		Toupper(&str);
		return str;
	}

	int String::toInt() const {
		return std::stoi(c_str());
	}
	float String::toFloat()const {
		return std::stof(c_str());
	}
	double String::toDouble()const {
		return std::stod(c_str());
	}
	int64_t String::toInt64() const {
		return std::stoll(c_str());
	}

	String String::trim()const {
		const char* raw = this->c_str();
		size_t totalLen = this->size();

		// 优化：直接使用 this 指针，避免复制
		if (totalLen == 0) {
			return String();
		}

		// 找起始位置
		size_t start = 0;
		while (start < totalLen && raw[start] == ' ') {
			++start;
		}

		// 如果全是空格
		if (start == totalLen) {
			return String();
		}

		// 找结束位置（最后一个非空格字符之后的位置）
		size_t end = totalLen;
		while (end > start && raw[end - 1] == ' ') {
			--end;
		}

		return String(raw + start, end - start);
	}
	size_t String::count(const String& value)const
	{
		if (value.empty()) {
			return 0;
		}
		size_t count = 0;
		size_t pos = 0;
		const size_t valueLen = value.size();
		// 优化：缓存 value.size() 避免重复调用
		while ((pos = this->find(value, pos)) != std::string::npos)
		{
			++count;
			pos += valueLen;
		}
		return count;
	}
	bool String::operator==(const wchar_t* szbuf)const
	{
		std::string u8str;
		UnicodeToUTF8(szbuf, &u8str);
		return (*this == u8str);
	}
	bool String::operator==(const std::wstring& wStr)const
	{
		std::string u8str;
		UnicodeToUTF8(wStr, &u8str);
		return (*this == u8str);
	}
	bool String::operator!=(const wchar_t* szbuf) const
	{
		std::string u8str;
		UnicodeToUTF8(szbuf, &u8str);
		return (*this != u8str);
	}
	bool String::operator!=(const std::wstring& wStr) const
	{
		std::string u8str;
		UnicodeToUTF8(wStr, &u8str);
		return (*this != u8str);
	}
	std::wstring String::unicode() const {
		std::wstring wstr;
		UTF8ToUnicode(*this, &wstr);
		return wstr;
	}
	std::string String::ansi() const {
		std::string str;
		UTF8ToANSI(*this, &str);
		return str;
	}
	String String::FromLocal(const std::string& localStr) {
		std::string u8Str;
		ANSIToUTF8(localStr, &u8Str);
		return u8Str;
	}
	String String::ToString(double number, size_t keepBitSize) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(keepBitSize) << number;
		return oss.str();
	}

	void AnyToUnicode(const std::string& src_str, UINT codePage, std::wstring* out_wstr) {
		if (src_str.empty()) {
			out_wstr->clear();
			return;
		}
		std::wstring& wstrCmd = *out_wstr;
		// 优化：一次性分配正确大小的内存
		int bytes = ::MultiByteToWideChar(codePage, 0, src_str.c_str(), (int)src_str.size(), NULL, 0);
		if (bytes > 0) {
			wstrCmd.resize(bytes);
			::MultiByteToWideChar(codePage, 0, src_str.c_str(), (int)src_str.size(), &wstrCmd[0], bytes);
		}
		else {
			wstrCmd.clear();
		}
	}
	void UnicodeToAny(const std::wstring& wstr, UINT codePage, std::string* out_str) {
		if (wstr.empty()) {
			out_str->clear();
			return;
		}
		std::string& strCmd = *out_str;
		// 优化：一次性分配正确大小的内存
		int bytes = ::WideCharToMultiByte(codePage, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
		if (bytes > 0) {
			strCmd.resize(bytes);
			::WideCharToMultiByte(codePage, 0, wstr.c_str(), (int)wstr.size(), &strCmd[0], bytes, NULL, NULL);
		}
		else {
			strCmd.clear();
		}
	}

	//以下是静态函数
	void ANSIToUniCode(const std::string& str, std::wstring* outStr)
	{
		AnyToUnicode(str, ::GetACP(), outStr);
	}
	void UnicodeToANSI(const std::wstring& wstr, std::string* outStr)
	{
		UnicodeToAny(wstr, ::GetACP(), outStr);
	}

	void GBKToUTF8(const std::string& str, std::string* outStr) {
		UINT gbkCodePage = 936;
		std::wstring wstr;
		AnyToUnicode(str, gbkCodePage, &wstr);
		UnicodeToUTF8(wstr, outStr);
	}

	void UTF8ToGBK(const std::string& str, std::string* outStr) {
		UINT gbkCodePage = 936;
		std::wstring wstr;
		UTF8ToUnicode(str, &wstr);
		UnicodeToAny(wstr, gbkCodePage, outStr);
	}

	void ANSIToUTF8(const std::string& str, std::string* outStr)
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
	void UTF8ToANSI(const std::string& str, std::string* outStr) {
		UINT codePage = ::GetACP();
		if (codePage == CP_UTF8) {
			*outStr = str;//如果本身就是utf8则不需要转换
			return;
		}
		std::wstring wstr;
		UTF8ToUnicode(str, &wstr);
		UnicodeToAny(wstr, codePage, outStr);
	}
	void UnicodeToUTF8(const std::wstring& wstr, std::string* outStr)
	{
		UnicodeToAny(wstr, CP_UTF8, outStr);
	}
	void UTF8ToUnicode(const std::string& str, std::wstring* outStr) {
		AnyToUnicode(str, CP_UTF8, outStr);
	}

	void Tolower(std::string* str_in_out)
	{
		std::string& str = *str_in_out;
		// 优化：直接修改字符串，避免 const_cast
		for (size_t i = 0; i < str.size(); ++i)
		{
			char& ch = str[i];
			if (ch >= 'A' && ch <= 'Z') {
				ch += 32;
			}
		}
	}
	void Toupper(std::string* str_in_out)
	{
		std::string& str = *str_in_out;
		// 优化：直接修改字符串，避免 const_cast
		for (size_t i = 0; i < str.size(); ++i)
		{
			char& ch = str[i];
			if (ch >= 'a' && ch <= 'z') {
				ch -= 32;
			}
		}
	}
	void Erase(std::string* str_in_out, char _char) {
		std::string& str = *str_in_out;
		// 优化：使用 erase-remove idiom，避免内存分配
		str.erase(std::remove(str.begin(), str.end(), _char), str.end());
	}
	size_t Replace(std::string* str_in_out, const std::string& oldText, const std::string& newText, bool replaceAll)
	{
		if (!str_in_out || oldText.empty()) return 0;
		size_t count = 0;
		std::string& str = *str_in_out;
		size_t pos = 0;
		const size_t oldLen = oldText.size();
		const size_t newLen = newText.size();
		// 优化：缓存字符串长度，避免重复调用
		while ((pos = str.find(oldText, pos)) != std::string::npos)
		{
			str.replace(pos, oldLen, newText);
			pos += newLen; // 跳过新插入的文本
			++count;
			if (!replaceAll) {
				break;
			}
		}
		return count;
	}
	void Replace(std::string* str_in_out, char oldChar, char newChar)
	{
		// 优化：直接替换，不需要迭代器
		std::string& str = *str_in_out;
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == oldChar) {
				str[i] = newChar;
			}
		}
	}

	template<typename T>
	void __Split(const std::string& str_in, const std::string& ch_, std::vector<T>* strs_out)
	{
		std::vector<T>& arr = *strs_out;
		arr.clear();
		if (str_in.empty() || ch_.empty()) return;

		// 优化：预估分割数量，预留空间减少内存重新分配
		size_t estimatedCount = 1;
		size_t pos = 0;
		while ((pos = str_in.find(ch_, pos)) != std::string::npos) {
			++estimatedCount;
			pos += ch_.size();
		}
		arr.reserve(estimatedCount);

		size_t start = 0;
		const size_t chLen = ch_.size();
		while ((pos = str_in.find(ch_, start)) != std::string::npos)
		{
			if (pos > start) {
				arr.emplace_back(str_in.substr(start, pos - start));
			}
			start = pos + chLen;
		}
		// 最后一个片段
		if (start < str_in.size()) {
			arr.emplace_back(str_in.substr(start));
		}
	}

	template<typename T>
	void __Split(const std::string& str_in, char ch, std::vector<T>* strs_out)
	{
		std::vector<T>& arr = *strs_out;
		arr.clear();
		if (str_in.empty()) return;

		// 优化：预估分割数量，预留空间
		size_t estimatedCount = 1 + std::count(str_in.begin(), str_in.end(), ch);
		arr.reserve(estimatedCount);

		size_t start = 0;
		size_t pos = 0;
		while ((pos = str_in.find(ch, start)) != std::string::npos)
		{
			if (pos > start) {
				arr.emplace_back(str_in.substr(start, pos - start));
			}
			start = pos + 1;
		}
		// 最后一个片段
		if (start < str_in.size()) {
			arr.emplace_back(str_in.substr(start));
		}
	}

	std::vector<String> String::split(const String& ch)const
	{
		std::vector<String> strs;
		__Split<String>(*this, ch, &strs);
		return strs;
	}
	std::vector<String> String::split(char ch) const
	{
		std::vector<String> strs;
		__Split<String>(*this, ch, &strs);
		return strs;
	}

	void Split(const std::string& str_in, const std::string& ch_, std::vector<std::string>* strs_out) {

		__Split<std::string>(str_in, ch_, strs_out);
	}
	//-----------------------------------------------Copy End-----------------------------------------------
};
