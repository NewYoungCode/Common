#pragma once
#include <Windows.h>
#include <map>

#include "Text.h"
#include "FileSystem.h"
#include "WinTool.h"
class SafeConfig
{
	Text::String fileName;
public:
	SafeConfig(const Text::String& fileName);
	Text::String ReadValue(const Text::String& key, const Text::String& defaultValue = "");
	Text::String ReadString(const Text::String& key, const Text::String& defaultValue = "");
	int ReadInt(const Text::String& key, int defaultValue = 0);
	void WriteValue(const Text::String& key, const Text::String& value);
};
