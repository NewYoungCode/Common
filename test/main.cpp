#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
int main() {
	//WebClient wc;
	//Text::Utf8String  resp;
	//wc.HttpGet("www.baidu.com", &resp);
	//printf("%s", resp.ansi().c_str());

	std::vector<FileSystem::FileInfo> result;
	FileSystem::Find("d:/test", result, "*.*", true);
	std::vector<Text::Utf8String> files;

	size_t heads = 0;
	for (auto& it : result) {
		if (it.FileType == FileSystem::File) {
			files.push_back(it.FullName);
		}
	}

	unsigned  __int64  headOffset = 0;
	auto sz = sizeof(__int64);

	File::Delete(L"d:/test.rc");
	std::ofstream ofs(L"d:/test.rc", std::ios::binary);
	ofs.seekp(sz);
	for (auto& it : files) {
		Text::Utf8String data;
		File::ReadFile(it, &data);
		ofs.write(data.c_str(), data.size());
		headOffset += data.size();
	}
	ofs.flush();
	ofs.close();

	return 0;
}