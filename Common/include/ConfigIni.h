#pragma once
#include "Common.h"
class ConfigIni {
protected:
	size_t buffSize = MAX_PATH;
	Text::Utf8String filename;
	Text::Utf8String section;
	DWORD GetValue(const Text::Utf8String& section, const Text::Utf8String& key, const Text::Utf8String& defaultValue, const Text::Utf8String& filename, Text::Utf8String& outResult)const;
	bool SetValue(const Text::Utf8String& section, const Text::Utf8String& key, const Text::Utf8String& Value, const Text::Utf8String& absoluteFilename)const;
public:
	ConfigIni();
	//FileName //一定要绝对路径
	ConfigIni(const Text::Utf8String& filename, const Text::Utf8String& defaultSection = "setting", bool create = true, size_t buffSize = MAX_PATH);
	void SetDefaultSection(const Text::Utf8String section);
	bool DeleteKey(const Text::Utf8String& key, const Text::Utf8String& section = "");
	//读取ini中的字符
	Text::Utf8String ReadString(const Text::Utf8String& key, const Text::Utf8String& defaultValue = "", const Text::Utf8String& section = "") const;
	//读取ini中的数字
	float ReadFloat(const Text::Utf8String& key, float defaultValue = 0, const Text::Utf8String& section = "") const;
	//读取ini中的int数字
	int ReadInt(const Text::Utf8String& key, int defaultValue = 0, const Text::Utf8String& section = "") const;
	//写入ini
	bool WriteValue(const Text::Utf8String& key, const Text::Utf8String& value, const Text::Utf8String& section = "")const;
	std::vector<Text::Utf8String> GetSections();
	void DeleteSection(const Text::Utf8String& section);
};
//
//class SafeConfigIni :public ConfigIni {
//private:
//	Text::Utf8String oldFilename;
//public:
//	//编码
//	static void EnCode(const Text::Utf8String &filename, const Text::Utf8String &outFilename) {
//		Text::Utf8String data;
//		File::ReadFile(filename, data);
//		char *memBytes = new char[data.size()];
//		size_t pos = 0;
//		for (auto&it : data) {
//			memBytes[pos] = it + 1;
//			pos++;
//		}
//		File::Delete(outFilename);
//		std::ofstream ofs(outFilename, std::ios::app | std::ios::binary);
//		ofs.write(memBytes, data.size());
//		ofs.flush();
//		ofs.close();
//		delete memBytes;
//	}
//	//解码
//	static void DeCode(const Text::Utf8String &filename, const Text::Utf8String &outFilename) {
//		Text::Utf8String data;
//		File::ReadFile(filename, data);
//		char *memBytes = new char[data.size()];
//		size_t pos = 0;
//		for (auto&it : data) {
//			memBytes[pos] = it - 1;
//			pos++;
//		}
//		File::Delete(outFilename);
//		std::ofstream ofs(outFilename, std::ios::app | std::ios::binary);
//		ofs.write(memBytes, data.size());
//		ofs.flush();
//		ofs.close();
//		delete memBytes;
//	}
//
//	SafeConfigIni(const Text::Utf8String &filename, const Text::Utf8String &defaultSection = "setting", bool create = true, size_t buffSize = MAX_PATH);
//};