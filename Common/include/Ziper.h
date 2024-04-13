#pragma once
#include "Text.h"
#include "FileSystem.h"

DECLARE_HANDLE(HZIP_Z);

/// <summary>
/// 专门负责压缩的库
/// </summary>
class Ziper {
	HZIP_Z ptr = NULL;
public:
	Ziper(const std::wstring& createFileName, const std::string& pwd = "");
	void AddFile(const std::wstring& showFileName, const std::wstring& localFileName);
	void AddFolder(const std::wstring& showFolder);
	void Close();
	virtual ~Ziper();
public:
	static void Zip(const Text::String& _dirName, const Text::String& outFileName, const Text::String& pwd = "", std::function<bool(const Text::String&, int, int)> callback = NULL) {
		std::vector<FileSystem::FileInfo> result;
		Ziper zip(outFileName.unicode(), pwd);
		Text::String dirName = _dirName + "/";
		dirName = dirName.Replace("\\", "/");
		dirName = dirName.Replace("//", "/");
		FileSystem::Find(dirName, result, "*.*", true);
		for (int i = 0; i < result.size(); i++)
		{
			Text::String ItemNmae = result[i].FullName.Replace(dirName, "");
			if (result[i].FileType == FileSystem::Directory) {
				zip.AddFolder(ItemNmae.unicode());
			}
			else {
				zip.AddFile(ItemNmae.unicode(), result[i].FullName.unicode());
			}
			if (callback) {
				if (callback(ItemNmae, i, result.size())) {
					break;
				}
			}
		}
	}
};

