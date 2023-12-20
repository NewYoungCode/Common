#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include <sstream>

void convert(const std::vector<Text::Utf8String>& files);

int main() {

	convert(Path::SearchFiles("D:/C++/Common/Common/include", "*.h"));
	convert(Path::SearchFiles("D:/C++/Common/Common/include", "*.hpp"));
	convert(Path::SearchFiles("D:/C++/Common/Common/sources", "*.cpp"));

	return 0;
}

void convert(const std::vector<Text::Utf8String>& files)
{
	for (auto& it : files) {
		std::string text;
		File::ReadFile(it, &text);
		std::string u8;
		Text::Utf8String::ANSIToUTF8(text, &u8);
		File::WriteFile(&u8, it);
	}
}
