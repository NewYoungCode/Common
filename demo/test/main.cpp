#include <sstream>
#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include "WinTool.h"
#include "JsonValue.h"
#include "QrenCode.hpp"
#include "Log.h"
//
//int main() {
//
//	Text::String linkName = L"C:\\Users\\ly\\Downloads";
//	Text::String target = L"D:\\down";
//
//	Text::String cmd = "cmd.exe /c mklink /j " + linkName + " " + target;
//	auto ret = WinTool::ExecuteCmdLine(cmd);
//
//	//新增openssl,zlib库
//
//	std::cout << "PCID:" << WinTool::GetComputerID() << std::endl;
//
//	//二维码的使用
//	//QrenCode::Generate("https://www.baidu.com", L"d:/aa.bmp");
//	auto bmp = QrenCode::Generate("https://www.baidu.com");
//	::DeleteObject(bmp);
//
//	WinTool::GetWinVersion();
//
//	JsonValue obj("aa");
//	Json::Value jv;
//
//	WebClient wc;
//
//	Text::String resp;
//	wc.HttpGet("https://songsearch.kugou.com/song_search_v2?platform=WebFilter&pagesize=20&page=1&keyword=dj", &resp);
//
//	auto w = resp.ansi();
//	std::cout << w;
//
//	obj = jv;
//
//	//获取com端口名称
//	auto coms = WinTool::GetComPorts();
//
//	return 0;
//}
//

#include <iostream>
#include <string>
#include <search.h>
#include <algorithm>
int main(int argc, char* argv[]) {
	Log::Enable = true;
	Log::WriteFile = true;
	std::string command = "fh_loader1.exe";  // 你要转发的 exe 名称
	// 把所有参数拼接成命令行
	for (int i = 1; i < argc; ++i) {
		command += " " + std::string(argv[i]);
	}
	Log::WriteLog(command);
	// 调用系统命令执行目标 exe
	int result = system(command.c_str());
	return result;
}



#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>

// 用于读取文件内容的函数
bool XReadFile(const std::string& filePath, std::vector<unsigned char>& outputData) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    outputData.resize(size);
    file.read(reinterpret_cast<char*>(&outputData[0]), size);
    file.close();
    return true;
}

// 用于写入文件的函数
bool XWriteFile(const std::string& filePath, const std::vector<unsigned char>& inputData) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(&inputData[0]), inputData.size());
    file.close();
    return true;
}

int main2(int argc, char* argv[]) {
    std::string input_filename = "D:\\vendor_boot.img";  // 输入文件路径
    std::vector<unsigned char> fileData;

    // 读取文件
    if (!XReadFile(input_filename, fileData)) {
        std::cerr << "Failed to open file: " << input_filename << std::endl;
        return 1;
    }

    // 输出部分读取的字节，验证读取是否正确
    std::cout << "First 16 bytes of the file for verification:" << std::endl;
    for (size_t i = 0; i < 16 && i < fileData.size(); ++i) {
        std::cout << std::hex << static_cast<int>(fileData[i]) << " ";
    }
    std::cout << std::endl;

    // 定义字节序列
    const unsigned char w_row[] = { 0x49, 0x52, 0x4F, 0x57 };  // "ROW"
    const unsigned char w_prc[] = { 0x49, 0x50, 0x52, 0x43 };  // "PRC"

    size_t pos = std::search(fileData.begin(), fileData.end(), std::begin(w_row), std::end(w_row)) - fileData.begin();
    std::vector<unsigned char> outData;
    std::string out_filename;

    if (pos != fileData.size()) {  // 找到 "ROW"
        std::cout << "change row -> prc" << std::endl;
        outData = fileData;
        for (size_t i = 0; i < outData.size() - 3; ++i) {
            if (outData[i] == w_row[0] && outData[i + 1] == w_row[1] &&
                outData[i + 2] == w_row[2] && outData[i + 3] == w_row[3]) {
                outData[i] = w_prc[0];
                outData[i + 1] = w_prc[1];
                outData[i + 2] = w_prc[2];
                outData[i + 3] = w_prc[3];
            }
        }
        out_filename = "vendor_boot_prc.img";
    }
    else {
        pos = std::search(fileData.begin(), fileData.end(), std::begin(w_prc), std::end(w_prc)) - fileData.begin();
        if (pos != fileData.size()) {  // 找到 "PRC"
            outData = fileData;
            std::cout << "change prc -> row" << std::endl;
            for (size_t i = 0; i < outData.size() - 3; ++i) {
                if (outData[i] == w_prc[0] && outData[i + 1] == w_prc[1] &&
                    outData[i + 2] == w_prc[2] && outData[i + 3] == w_prc[3]) {
                    outData[i] = w_row[0];
                    outData[i + 1] = w_row[1];
                    outData[i + 2] = w_row[2];
                    outData[i + 3] = w_row[3];
                }
            }
            out_filename = "vendor_boot_row.img";
        }
        else {
            std::cerr << "Neither \"ROW\" nor \"PRC\" found" << std::endl;
            return 1;
        }
    }

    // 写入修改后的文件
    if (!XWriteFile(out_filename, outData)) {
        std::cerr << "Failed to write the output file: " << out_filename << std::endl;
        return 1;
    }

    std::cout << "Output file: " << out_filename << std::endl;

    // 保留 a 和 b 变量来计算 MD5 (假设你的函数已经实现了)
    std::string a = Util::MD5FromFile(L"D:\\python_vendor_boot_prc.img");
    std::string b = Util::MD5FromFile(L"D:\\vendor_boot_prc.img");

    // 输出计算出来的 MD5 值
    std::cout << "MD5 of the file D:\\python_vendor_boot_prc.img: " << a << std::endl;
    std::cout << "MD5 of the file D:\\vendor_boot_prc.img: " << b << std::endl;

    return 0;
}
