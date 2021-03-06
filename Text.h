#pragma once
#include <string>
#include <vector>
#include <Windows.h>
namespace Text {
	//������д����ĸתСд
	std::string ToLower(const std::string&str);
	//����Сд����ĸת��д
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
	//��ȡ�ַ���(���ַ�)
	std::wstring substr(const std::wstring &str, size_t starIndex, size_t count = size_t(-1));
	//��ȡ�ַ���(խ�ַ�)
	std::string substr(const std::string &str, size_t starIndex, size_t count = size_t(-1));
	//�滻�ַ����������µ��ַ���
	std::string ReplaceAll(const std::string &str, const std::string &oldText, const std::string &newText);
	//ֱ���滻�ַ����ҷ����޸ĸ���
	size_t Replace(std::string &str, const std::string & oldText, const std::string & newText);
	//�ַ����ָ�
	std::vector<std::string> Split(const std::string& str, const std::string& ch);


	class String //�Լ�д���ַ����������
	{
	private:
		char* _c_str = NULL;
		size_t _count = 0;
		//�����ڴ�
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
