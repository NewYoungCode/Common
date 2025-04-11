#include "test.h"

namespace test {
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


	std::wstring findCommandLine(const std::wstring& exeName) {
		HRESULT hres;

		static bool initialized = false;
		static IWbemServices* pSvc = nullptr;

		if (!initialized) {
			hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			if (FAILED(hres)) return L"[CoInit failed]";

			hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE, NULL);
			if (FAILED(hres)) return L"[Security init failed]";

			IWbemLocator* pLoc = NULL;
			hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
				IID_IWbemLocator, (LPVOID*)&pLoc);
			if (FAILED(hres)) return L"[Create locator failed]";

			hres = pLoc->ConnectServer(
				_bstr_t(L"ROOT\\CIMV2"),
				NULL, NULL, 0, NULL, 0, 0, &pSvc);
			pLoc->Release();
			if (FAILED(hres)) return L"[ConnectServer failed]";

			hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
				NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE);
			if (FAILED(hres)) {
				pSvc->Release();
				return L"[Proxy blanket failed]";
			}

			//initialized = true;
		}

		// 注意：LIKE 用于大小写不敏感匹配
		std::wstring query = L"SELECT CommandLine FROM Win32_Process WHERE Name LIKE '" + exeName + L"'";

		IEnumWbemClassObject* pEnumerator = nullptr;
		HRESULT hr = pSvc->ExecQuery(
			bstr_t("WQL"),
			bstr_t(query.c_str()),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&pEnumerator);

		if (FAILED(hr)) return L"[Query failed]";

		IWbemClassObject* pclsObj = NULL;
		ULONG uReturn = 0;

		std::wstring cmdline = L"";

		while (pEnumerator) {
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (uReturn == 0) break;

			VARIANT vtProp;
			hr = pclsObj->Get(L"CommandLine", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
				cmdline = vtProp.bstrVal;
				VariantClear(&vtProp);
				pclsObj->Release();
				break; // 只取第一个匹配项
			}
			VariantClear(&vtProp);
			pclsObj->Release();
		}

		pEnumerator->Release();
		CoUninitialize(); // 可选，因为程序不会退出
		return cmdline.empty() ? L"[Not Found]" : cmdline;
	}
}


////驱动文件安装
//
//#include <windows.h>
//#include <newdev.h>
//#include <iostream>
//#pragma comment(lib, "newdev.lib")  // 链接 newdev 库
//int main3() {
//	// 指向你的驱动 INF 文件路径
//	LPCWSTR infPath = L"D:\\平板文件\\新建文件夹\\android_winusb.inf";
//	BOOL rebootRequired = FALSE;
//	// 调用安装函数
//	if (DiInstallDriver(NULL, infPath, DIIRFLAG_FORCE_INF, &rebootRequired)) {
//		std::wcout << L"驱动安装成功。" << std::endl;
//		if (rebootRequired) {
//			std::wcout << L"需要重启系统以完成安装。" << std::endl;
//		}
//	}
//	else {
//		std::wcout << L"驱动安装失败。错误码：" << GetLastError() << std::endl;
//	}
//	return 0;
//}

