#include "os.h"
#include "WebClient.h"
int main(int count, char* args[]) {

	std::string resp;
	WebClient wc;

	wc.HttpGet("www.baidu.com", resp);

	WinTool::SetAutoBoot(Path::StartFileName(), true);

	return 0;

}