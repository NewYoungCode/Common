#include "Text.h"

//¶¨Òå................................................
namespace Text {
	std::string ToUpper(const std::string&str) {
		char* cStr = (char*)malloc(str.size() + 1);
		size_t pos = 0;
		for (auto &ch : str) {
			char newCh = ch;
			if (ch >= 97 && ch <= 122) { 
				newCh = ch - 32;
			}
			cStr[pos] = newCh;
			pos++;
		}
		cStr[str.size()] = 0;
		std::string newStr = cStr;
		free(cStr);
		return newStr;
	}
	std::string ToLower(const std::string&str) {
		char* cStr = (char*)malloc(str.size() + 1);
		size_t pos = 0;
		for (auto &ch : str) {
			char newCh = ch;
			if (ch >= 65 && ch <= 90) {
				newCh = ch + 32;
			}
			cStr[pos] = newCh;
			pos++;
		}
		cStr[str.size()] = 0;
		std::string newStr = cStr;
		free(cStr);
		return newStr;
	}
	std::string ReplaceAll(const std::string &str, const std::string & oldText, const std::string & newText)
	{
		std::string newStr = str;
		size_t pos;
		pos = str.find(oldText);
		while (pos != std::string::npos) {
			newStr.replace(pos, oldText.size(), newText);
			pos = newStr.find(oldText);
		}
		return newStr;
	}
	size_t  Replace(std::string  &str, const std::string & oldText, const std::string & newText)
	{
		std::string &newStr = str;
		size_t pos;
		pos = str.find(oldText);
		size_t count = 0;
		for (; pos != std::string::npos;) {
			newStr.replace(pos, oldText.size(), newText);
			count++;
			pos = newStr.find(oldText);
		}
		return count;
	}
	std::vector<std::string> Split(const std::string& str, const std::string& ch_) {
		std::vector<std::string> arr;
		std::string buf = str;
		size_t pos = buf.find(ch_);
		if (pos == size_t(-1)) {
			arr.push_back(str);
			return arr;
		}
		for (; pos != std::string::npos;) {
			arr.push_back(buf.substr(0, pos));
			buf = buf.erase(0, pos + ch_.size());
			pos = buf.find(ch_);
			if (pos == std::string::npos) {
				arr.push_back(buf);
			}
		}
		return arr;
	}
	std::wstring substr(const std::wstring &str, size_t starIndex, size_t count) {
		return str.substr(starIndex, count);
	}
	std::string substr(const std::string &str, size_t starIndex, size_t count) {
		std::wstring newStr = Text::ANSIToUniCode(str).substr(starIndex, count);
		return Text::UnicodeToANSI(newStr);
	}
	std::wstring ANSIToUniCode(const std::string &str)
	{
		int bytes = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
		std::wstring wstrCmd;
		wstrCmd.resize(bytes);
		bytes = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), const_cast<wchar_t*>(wstrCmd.c_str()), wstrCmd.size());
		return wstrCmd;
	}
	std::string UnicodeToANSI(const std::wstring &wstr)
	{
		int bytes = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		std::string strCmd;
		strCmd.resize(bytes);
		bytes = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), const_cast<char*>(strCmd.data()), strCmd.size(), NULL, NULL);
		return strCmd;
	}
	std::string UnicodeToUTF8(const std::wstring &wstr)
	{
		int bytes = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		std::string strCmd;
		strCmd.resize(bytes);
		bytes = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), const_cast<char*>(strCmd.data()), strCmd.size(), NULL, NULL);
		return strCmd;
	}
	std::string UTF8ToANSI(const std::string& str)
	{
		int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		memset(pwBuf, 0, nwLen * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
		int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
		char* pBuf = new char[nLen + 1];
		memset(pBuf, 0, nLen + 1);
		WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
		std::string strRet = pBuf;

		delete[]pBuf;
		delete[]pwBuf;
		pBuf = NULL;
		pwBuf = NULL;
		return strRet;
	}
	std::string ANSIToUTF8(const std::string& str)
	{
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		ZeroMemory(pwBuf, nwLen * 2 + 2);
		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
		char* pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);
		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
		std::string strRet(pBuf);
		delete[]pwBuf;
		delete[]pBuf;
		pwBuf = NULL;
		pBuf = NULL;
		return strRet;
	}
}

namespace Text {
	String::String() {}
	String::String(const String&str) {
		this->malloc(str.Data(), str.Size());
	}
	String::String(const char*str) {
		this->malloc(str, std::strlen(str));
	}
#ifdef __cplusplus
	String::String(const std::string&str) {
		this->malloc(str.c_str(), str.size());
	}
#endif
	String::~String() {
		Clear();
	}
	void String::malloc(const char*str, size_t count) {
		Clear();
		_c_str = (char*)std::malloc(count + 1);
		_c_str[count] = 0;
		memcpy(_c_str, str, count);
		this->_count = count;
	}
	bool String::Equal(const char*str) const {
		if (std::strlen(str) == _count) {
			for (size_t i = 0; i < _count; i++)
			{
				if (_c_str[i] != str[i]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool String::Equal(const String&str) const {
		return Equal(str.Data());
	}
	size_t String::Size() const {
		return _count;
	}
	void String::Clear() {
		if (_c_str) {
			free(_c_str);
			_c_str = NULL;
			_count = 0;
		}
	}
	bool String::Empty() const {
		return _c_str == 0 ? true : false;
	}
	const char* String::Data()const {
		return this->_c_str;
	}
	String& String::operator=(const String&str) {
		this->malloc(str.Data(), str.Size());
		return *this;
	}
	String& String::operator+=(const String&str) {
		return Concat(str.Data(), str.Size());
	}
	String& String::operator+=(const char*str) {
		return Concat(str, std::strlen(str));
	}
	String& String::Concat(const char*str, size_t count) {
		size_t size_ = Size() + count;
		char *newStr = (char*)std::malloc(size_ + 1);
		newStr[size_] = 0;
		size_t pos = 0;
		for (size_t i = 0; i < Size(); i++, pos++)
		{
			newStr[pos] = _c_str[i];
		}
		for (size_t i = 0; i < count; i++, pos++)
		{
			newStr[pos] = str[i];
		}
		Clear();
		_c_str = newStr;
		_count = size_;
		return *this;
	}
	String& String::Replace(const String&oldText, const String&newText) {
		size_t pos = this->Find(oldText);
		if (pos != size_t(-1)) {
			//char *begin=
		}
		return *this;
	}
	String& String::ToLower() {
		for (size_t i = 0; i < Size(); i++)
		{
			if (_c_str[i] >= 65 && _c_str[i] <= 90) {
				_c_str[i] = _c_str[i] + 32;
			}
		}
		return *this;
	}
	String& String::ToUpper() {
		for (size_t i = 0; i < Size(); i++)
		{
			if (_c_str[i] >= 97 && _c_str[i] <= 122) {
				_c_str[i] = _c_str[i] - 32;
			}
		}
		return *this;
	}
	bool String::operator==(const String &str) const {
		return Equal(str.Data());
	}
	bool String::operator!=(const String &str) const {
		return !Equal(str.Data());
	}
	size_t String::Find(const String&text) const {
		size_t pos = -1;
		for (size_t i = 0; i < Size(); i++)
		{
			if (_c_str[i] == *text.Data()) {
				bool find = true;
				char *chPtr = &_c_str[i];
				if (this->Size() - i < text.Size()) {
					break;
				}
				for (size_t j = 0; j < text.Size(); j++)
				{
					if (find && chPtr[j] != text.Data()[j]) {
						find = false;
						break;
					}
				}
				if (find) {
					pos = i;
					break;
				}
			}
		}
		return pos;
	}
	String String::SubString(size_t pos, size_t count) const {
		if (count > this->Size()) {
			count = this->Size() - pos;
		}
		char *outStr = (char*)std::malloc(count + 1);
		outStr[count] = 0;
		for (size_t i = pos, j = 0; j < count; i++, j++)
		{
			outStr[j] = this->Data()[i];
		}
		String newStr = outStr;
		std::free(outStr);
		return newStr;
	}
}
