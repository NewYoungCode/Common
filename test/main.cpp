#include <sstream>

#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include "WinTool.h"
#include "JsonValue.h"
int main() {

	std::cout << "PCID:" << WinTool::GetComputerID() << std::endl;

	WinTool::GetWinVersion();

	JsonValue obj("aa");
	Json::Value jv;

	WebClient wc;

	Text::String resp;
	wc.HttpGet("https://songsearch.kugou.com/song_search_v2?platform=WebFilter&pagesize=20&page=1&keyword=dj", &resp);

	auto w = resp.ansi();
	std::cout << w;

	obj = jv;

	return 0;
}
