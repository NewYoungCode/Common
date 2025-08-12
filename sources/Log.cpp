#include "Log.h"
namespace Log {
	//是否启用日志
	bool WriteFile = true;
	void WriteLog(const Text::String& log)
	{
		if (!WriteFile)return;
#ifdef _DEBUG
		Text::String logPath = Path::StartPath() + "\\" + Path::GetFileNameWithoutExtension(Path::StartFileName()) + "_Log";
#else
		Text::String logPath = Path::GetAppTempPath() + "_Log";
#endif
		Directory::Create(logPath);
		Text::String logFile = logPath + "\\" + Time::Now().ToString("yyyy-MM-dd") + ".log";
		std::ofstream ofs(logFile.unicode(), std::ios::binary | std::ios::app);
		ofs.write(log.c_str(), log.size());
		ofs.flush();
		ofs.close();
	}
};

