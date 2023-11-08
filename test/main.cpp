#include "WebClient.h"
int main() {
	WebClient wc;
	Text::Utf8String  resp;
	wc.HttpGet("www.baidu.com", &resp);
	printf("%s", resp.ansi().c_str());
	return 0;
}