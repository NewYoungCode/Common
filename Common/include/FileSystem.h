#pragma once
#include <sys/stat.h>
#include <direct.h>
#include <functional>
#include <fstream>
#include "Text.h"
#include "Md5.h"
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
	//д���ļ�
	extern void WriteFile(const char* fileStream, size_t count, const Text::Utf8String& filename);
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
	//����·��
	extern bool Create(const Text::Utf8String& path);
	//����Ŀ¼�����ļ���Ŀ��Ŀ¼
	extern bool Copy(const Text::Utf8String& srcPath, const Text::Utf8String& desPath);
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
	extern const Text::Utf8String& StartFileName();
#undef GetTempPath
	/// <summary>
	/// ��ȡӦ��ǰwindows�û�����ʱĿ¼
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetTempPath();
	/// <summary>
	/// ��ȡӦ�ó������ʱĿ¼
	/// </summary>
	/// <returns></returns>
	extern Text::Utf8String GetAppTempPath();
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
	public:
		FileType FileType = FileSystem::FileType::None;
		Text::Utf8String Extension;
		Text::Utf8String FullName;
		Text::Utf8String FileName;
		ULONGLONG FileSize = 0;
		bool ReadOnly = false;
		FileInfo(const Text::Utf8String& fileName) {
			if (File::Exists(fileName)) {
				Extension = Path::GetExtension(fileName);
				FileName = Path::GetFileName(fileName);
				FullName = fileName;
				FileType = FileType::File;
				std::ifstream ifs(fileName.unicode(), std::ios::binary);
				ifs.seekg(0, std::ios::end);
				FileSize = ifs.tellg();
			}
		}
		FileInfo() {}
		void Close() {
		}
		~FileInfo() {
		}
	};
	extern size_t Find(const Text::Utf8String& directory, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern = "*.*", bool loopSubDir = false);
}