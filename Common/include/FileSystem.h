#pragma once
#include <sys/stat.h>
#include <direct.h>
#include <functional>
#include <fstream>
#include "Text.h"
namespace File {
	typedef std::string FileStream;
	//�����ļ�
	extern bool Create(const Text::Utf8String& filename);
	//ɾ���ļ�
	extern bool Delete(const Text::Utf8String& filename);
	//�ж��ļ��Ƿ����
	extern bool Exists(const Text::Utf8String& filename);
	//�ļ��ƶ����߸���
	extern bool Move(const Text::Utf8String& oldname, const Text::Utf8String& newname);
	//��ȡ�ļ���out����
	extern void ReadFile(const  Text::Utf8String& filename, FileStream* fileStream);
	//д���ļ�
	extern void WriteFile(const FileStream* fileStream, const Text::Utf8String& filename);
	//�����ļ�
	extern void Copy(const  Text::Utf8String& filename, const  Text::Utf8String& des_filename);
}
namespace Path {
	//�Լ�д���ļ������
	class FileWatcher {
	private:
		Text::Utf8String math = "*.*";
		Text::Utf8String path;
		std::function<void(const Text::Utf8String& filename)> callback = NULL;
		size_t sleep;
		bool ctn = true;
		void TaskFunc();
	public:
		FileWatcher(const Text::Utf8String& path, const Text::Utf8String& math, const std::function<void(const Text::Utf8String& filename)>& callback, size_t sleep = 500);
		~FileWatcher();
	};
	//����·��  MultiDir:�Ƿ񴴽��༶Ŀ¼
	extern bool Create(const Text::Utf8String& path);
	//ɾ��·�� ����������ļ��л����ļ� ����ݹ�ɾ��
	extern bool Delete(const Text::Utf8String& directoryName);
	//ͨ��������ļ�
	extern std::vector<Text::Utf8String> SearchFiles(const Text::Utf8String& path, const Text::Utf8String& pattern);
	//���·���Ƿ����
	extern bool Exists(const Text::Utf8String& path);
	//��ȡ�ļ�����(�ļ�����)
	extern Text::Utf8String GetFileNameWithoutExtension(const Text::Utf8String& _filename);
	//��ȡ�ļ�Ŀ¼����(����Ŀ¼)
	extern Text::Utf8String GetDirectoryName(const Text::Utf8String& _filename);
	//��ȡ�ļ�����+��׺
	extern Text::Utf8String GetFileName(const Text::Utf8String& _filename);
	//��ȡ�ļ���׺��(��׺��)
	extern Text::Utf8String GetExtension(const Text::Utf8String& _filename);
	//��ȡ�������ھ���·��Ŀ¼
	extern Text::Utf8String StartPath();
	//��ȡ�������ھ���·�������ļ�����
	extern Text::Utf8String StartFileName();
#undef GetTempPath
	/// <summary>
	/// ��ȡӦ�ó������ʱĿ¼
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetTempPath();
	/// <summary>
	/// ��ȡӦ�ó������ݴ洢Ŀ¼ C:/Users/%s/AppData/Local/%s
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetAppDataPath();
};
namespace FileSystem {
	typedef enum : char {
		None,
		File,
		Directory
	}FileType;
	struct FileInfo
	{
	private:
		std::ifstream* fs = NULL;
	public:
		unsigned long long StreamPos = 0;
		struct _stat64 __stat;
		FileType FileType = FileSystem::FileType::None;
		Text::Utf8String Extension;
		Text::Utf8String FullName;
		Text::Utf8String FileName;
		bool ReadOnly = false;
		size_t Read(char* _buf_, size_t _rdCount = 512) {
			size_t rdbufCount = _rdCount;
			if (StreamPos + _rdCount >= __stat.st_size) {
				rdbufCount = __stat.st_size - StreamPos;
			}
			if (rdbufCount == 0) {
				return 0;
			}
			if (fs == NULL) {
				fs = new std::ifstream(FullName.utf16(), std::ios::binary);
			}
			fs->seekg(StreamPos);
			fs->read(_buf_, rdbufCount);
			StreamPos += rdbufCount;
			return rdbufCount;
		}
		FileInfo() {}
		FileInfo(const Text::Utf8String& filename) {
			int status = _wstat64(filename.utf16().c_str(), &__stat);
			if (status == 0 && (__stat.st_mode & S_IFREG) == S_IFREG) {
				Extension = Path::GetExtension(filename);
				FileName = Path::GetFileName(filename);
				FullName = filename;
				FileType = FileType::File;
			}
		}
		void Close() {
			if (fs) {
				fs->close();
				delete fs;
				fs = NULL;
			}
		}
		~FileInfo() {
			if (fs) {
				fs->close();
				delete fs;
			}
		}
	};
	extern void ReadFileInfoWin32(const Text::Utf8String& directory, WIN32_FIND_DATAW& pNextInfo, std::vector<FileSystem::FileInfo>& result);
	extern size_t  Find(const Text::Utf8String& directory, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern = "*.*");
}