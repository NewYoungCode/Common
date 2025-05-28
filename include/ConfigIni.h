#pragma once
#include <Windows.h>
#include <iostream>
#include "Text.h"
#include "FileSystem.h"
class ConfigIni {
protected:
	size_t buffSize = 512;
	Text::String filename;
	Text::String section;
	DWORD GetValue(const Text::String& section, const Text::String& key, const Text::String& defaultValue, const Text::String& filename, Text::String& outResult)const;
	bool SetValue(const Text::String& section, const Text::String& key, const Text::String& Value, const Text::String& absoluteFilename)const;
private:
	ConfigIni() = delete;
	ConfigIni(const ConfigIni&) = delete;
public:
	//FileName //一定要绝对路径
	ConfigIni(const Text::String& filename, const Text::String& defaultSection = "setting", size_t buffSize = 512);
	void SetDefaultSection(const Text::String section);
	//读取ini中的字符
	Text::String ReadString(const Text::String& key, const Text::String& defaultValue = "", const Text::String& section = "") const;
	//读取ini中的数字
	float ReadFloat(const Text::String& key, float defaultValue = 0, const Text::String& section = "") const;
	//读取ini中的int数字
	int ReadInt(const Text::String& key, int defaultValue = 0, const Text::String& section = "") const;
	//写入ini
	bool WriteValue(const Text::String& key, const Text::String& value, const Text::String& section = "")const;
	//获取所有的Section
	std::vector<Text::String> GetSections();
	//删除所有的Section
	void DeleteSection(const Text::String& section);
};