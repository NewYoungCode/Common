#include <sstream>
#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include "WinTool.h"
#include "JsonValue.h"
#include "QrenCode.hpp"

int main() {

	Text::String linkName = L"C:\\Users\\ly\\Downloads";
	Text::String target = L"D:\\down";

	Text::String cmd = "cmd.exe /c mklink /j " + linkName + " " + target;
	auto ret = WinTool::ExecuteCmdLine(cmd);

	//新增openssl,zlib库

	std::cout << "PCID:" << WinTool::GetComputerID() << std::endl;

	//二维码的使用
	//QrenCode::Generate("https://www.baidu.com", L"d:/aa.bmp");
	auto bmp = QrenCode::Generate("https://www.baidu.com");
	::DeleteObject(bmp);

	WinTool::GetWinVersion();

	JsonValue obj("aa");
	Json::Value jv;

	WebClient wc;

	Text::String resp;
	wc.HttpGet("https://songsearch.kugou.com/song_search_v2?platform=WebFilter&pagesize=20&page=1&keyword=dj", &resp);

	auto w = resp.ansi();
	std::cout << w;

	obj = jv;

	//获取com端口名称
	auto coms = WinTool::GetComPorts();

	return 0;
}

