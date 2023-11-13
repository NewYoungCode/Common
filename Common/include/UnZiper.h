#pragma once
#include "Text.h"
#include "FileSystem.h"

DECLARE_HANDLE(HZIP_U);

struct ZipItem
{
	int index;                 // index of this file within the zip
	TCHAR name[MAX_PATH];      // filename within the zip
	DWORD attr;                // attributes, as in GetFileAttributes.
	FILETIME atime, ctime, mtime;// access, create, modify filetimes
	long comp_size;            // sizes of item, compressed and uncompressed. These
	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
	bool isDir()const {
		if (attr & FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}
		return false;
	}
};

/// <summary>
/// 专门负责解压缩的类
/// </summary>
class UnZiper {
	HZIP_U ptr = NULL;
	int itemCount = 0;
public:
	UnZiper(const std::wstring& fileName, const std::string& password = "");
	UnZiper(const char* fileData, unsigned int size, const std::string& password = "");
	bool Find(const std::string& itemName, ZipItem* item);
	bool Find(int index, ZipItem* item);
	bool UnZipItem(const ZipItem& item, void** data);
	int GetCount();
	virtual ~UnZiper();
public:
	static void UnZip(const Text::Utf8String& zipFileName, const Text::Utf8String& outDir, const std::string& password = "", std::function<bool(const Text::Utf8String&, int, int)> callback = NULL) {
		Path::Create(outDir);
		UnZiper zip(zipFileName.unicode(), password);
		for (int i = 0; i < zip.GetCount(); i++)
		{
			ZipItem ze;
			zip.Find(i, &ze);
			Text::Utf8String itemName = outDir + "/" + Text::Utf8String(ze.name);
			if (ze.isDir()) {
				Path::Create(itemName);
			}
			else {
				File::Delete(itemName);
				void* data = NULL;
				zip.UnZipItem(ze, &data);
				std::ofstream ofs(itemName.unicode(), std::ios::binary);
				ofs.write((char*)data, ze.unc_size);
				ofs.flush();
				ofs.close();
				if (data) {
					delete data;
				}
			}
			if (callback) {
				if (callback(itemName, i, zip.GetCount())) {
					break;
				}
			}
		}
	}
};
