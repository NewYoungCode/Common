#include "Log.h"
namespace Log {
	//是否启用日志
	bool Enable = false;
	std::mutex __logMtx;
	void WriteLog(const Text::Utf8String& log)
	{
		if (!Enable)return;
#ifdef _DEBUG
		Text::Utf8String logPath = Path::StartPath() + "\\" + Path::GetFileNameWithoutExtension(Path::StartFileName()) + "_Log";
#else
		Text::Utf8String logPath = Path::GetTempPath() + "_Log";
#endif
		Path::Create(logPath);
		Text::Utf8String logFile = logPath + "\\" + Time::Now::ToString("yyyy-MM-dd") + ".txt";
		std::ofstream ofs(logFile.utf16(), std::ios::binary | std::ios::app);
		ofs.write(log.c_str(), log.size());
		ofs.flush();
		ofs.close();
	}
};

