#pragma once
#include <string>
#include <vector>
#include <Windows.h>
namespace Text {
	//包含大写的字母转小写
	std::string ToLower(const std::string&str);
	//包含小写的字母转大写
	std::string ToUpper(const std::string&str);
	//ANSIToUniCode
	std::wstring ANSIToUniCode(const std::string &strCmd);
	//UnicodeToANSI
	std::string UnicodeToANSI(const std::wstring &wstrCmd);
	//UnicodeToUTF8
	std::string UnicodeToUTF8(const std::wstring &wstrCmd);
	//ANSIToUTF8
	std::string ANSIToUTF8(const std::string &str);
	//UTF8ToANSI
	std::string UTF8ToANSI(const std::string &str);
	//截取字符串(宽字符)
	std::wstring substr(const std::wstring &str, size_t starIndex, size_t count = size_t(-1));
	//截取字符串(窄字符)
	std::string substr(const std::string &str, size_t starIndex, size_t count = size_t(-1));
	//替换字符串并返回新的字符串
	std::string ReplaceAll(const std::string &str, const std::string &oldText, const std::string &newText);
	//直接替换字符并且返回修改个数
	size_t Replace(std::string &str, const std::string & oldText, const std::string & newText);
	//字符串分割
	std::vector<std::string> Split(const std::string& str, const std::string& ch);


	class String //自己写的字符串方便操作
	{
	private:
		char* _c_str = NULL;
		size_t _count = 0;
		//申请内存
		void malloc(const char*str, size_t count);
	public:
		String();
		String(const String&str);
		String(const char*str);
#ifdef __cplusplus
		String(const std::string&str);
#endif
		~String();
		String& operator=(const String&str);
		String& operator+=(const String&str);
		String& operator+=(const char*str);
		String& Concat(const char*str, size_t count);
		bool operator==(const String &str) const;
		bool operator!=(const String &str) const;
		bool Equal(const char*str) const;
		bool Equal(const String&str) const;
		bool Empty() const;
		void Clear();
		friend std::ostream& operator<<(std::ostream &out_, const String&str) {
			out_ << str._c_str;
			return out_;
		}
		size_t Size() const;
		const char* Data()const;
		String& ToUpper();
		String& ToLower();
		String& Replace(const String&oldText, const String&newText);
		size_t Find(const String&text) const;
		String SubString(size_t index, size_t len = size_t(-1)) const;
	};
}
