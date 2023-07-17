#pragma once
#include "Text.h"
#include "FileSystem.h"
#include "WinTool.h"
class SafeConfig
{
	Text::Utf8String fileName;
public:
	SafeConfig(const Text::Utf8String& fileName);
	Text::Utf8String ReadValue(const Text::Utf8String& key, const Text::Utf8String& defaultValue = "");
	Text::Utf8String ReadString(const Text::Utf8String& key, const Text::Utf8String& defaultValue = "");
	int ReadInt(const Text::Utf8String& key, int defaultValue = 0);
	void WriteValue(const Text::Utf8String& key, const Text::Utf8String& value);
};
