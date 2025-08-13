#include "WebClient.h"

#if USECURL

#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif // !CURL_STATICLIB
#include "curl/curl.h"
#include "curl/easy.h"
#pragma comment(lib,"Crypt32.lib")//curl需要的库
#pragma comment(lib,"wldap32.lib")//curl需要的库
#pragma comment(lib,"ws2_32.lib") //curl需要的库

#ifdef  _WIN64

#ifdef  _DEBUG
#pragma comment (lib,"X64/libcurld.lib")
#else
#pragma comment (lib,"X64/libcurl.lib")
#endif

#else

#ifdef _DEBUG
#pragma comment(lib,"libcurld.lib")
#else
#pragma comment(lib,"libcurl.lib")
#endif // !_DEBUG

#endif

//curl的初始化
bool g_curl_bInit = false;
std::mutex g_curl_mtx;

struct StructCallback
{
	std::function<void(long long dltotal, long long dlnow)> progressCallback;
	ULONGLONG lastUpdate = 0;
};

//接收响应body
size_t g_curl_write_callback(char* contents, size_t size, size_t nmemb, void* respone);
//接受上传或者下载进度
int g_curl_progress_callback(void* ptr, __int64 dltotal, __int64 dlnow, __int64 ultotal, __int64 ulnow);

//定义
WebClient::WebClient() {
	{
		g_curl_mtx.lock();
		if (!g_curl_bInit) {
			CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
			if (code == CURLE_OK) {
				g_curl_bInit = true;
			}
		}
		g_curl_mtx.unlock();
	}
}

void WebClient::Cancel() {
	content.cancel = true;
}

WebClient::~WebClient() {
}

size_t g_curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	size_t count = size * nmemb;
	auto* ct = (WebClient::Content*)userdata;
	do
	{
		if (ct == NULL) {
			break;
		}
		if (ct->cancel) {//已取消请求
			return 0;
		}
		if (ct->type == 0) { //基本请求
			auto* response = (std::string*)ct->tag;
			response->append(ptr, count);
			break;
		}
		if (ct->type == 1) { //文件下载
			auto* ofs = (std::ofstream*)ct->tag;
			ofs->write(ptr, count);
			ofs->flush();
			break;
		}
	} while (false);
	return count;
};

int g_curl_progress_callback(void* ptr, __int64 dltotal, __int64 dlnow, __int64 ultotal, __int64 ulnow)
{
	if (!ptr || dltotal <= 0) return 0;

	StructCallback* cb = static_cast<StructCallback*>(ptr);
	ULONGLONG tick = ::GetTickCount64();
	bool isComplete = (dlnow >= dltotal);

	if (!isComplete && tick - cb->lastUpdate < 100) {
		return 0; // 限制 100ms 更新一次
	}
	cb->lastUpdate = tick;

	if (cb->progressCallback) {
		cb->progressCallback(dltotal, dlnow);
	}
	return 0;
}
CURL* WebClient::Init(const std::string& strUrl, std::string* strResponse, int nTimeout) {
	if (strResponse) {
		strResponse->clear();
	}
	CURL* curl = curl_easy_init();
	if (!curl) {
		return curl;
	}

	this->content.type = 0;
	this->content.cancel = false;
	this->content.tag = strResponse;
	if (!Proxy.empty()) {
		curl_easy_setopt(curl, CURLOPT_PROXY, Proxy.c_str()); //代理服务器地址
	}

	//初始化cookie引擎
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");    //初始化cookie引擎,才能正确接收到cookie数据.
	if (!Cookies.empty()) {
		curl_easy_setopt(curl, CURLOPT_COOKIE, Cookies.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);	// 验证对方的SSL证书
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);	//根据主机验证证书的名称
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());//设置url地址
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);//设置超时
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->content);//
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, g_curl_write_callback);//接受回调

	for (auto& it : Header) {
		auto hd = it.first + ":" + it.second;
		curl_header = curl_slist_append((curl_slist*)curl_header, hd.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, (curl_slist*)curl_header);
	return curl;
};
long WebClient::CleanUp(void* curl, int code) {
	long RESPONSE_CODE = (int)code;
	//如果执行成功,
	if (code == CURLE_OK)
	{
		//获取响应的状态码
		curl_easy_getinfo((CURL*)curl, CURLINFO_RESPONSE_CODE, &RESPONSE_CODE);

		struct curl_slist* cookies = NULL;
		curl_easy_getinfo((CURL*)curl, CURLINFO_COOKIELIST, &cookies);       //获得cookie数据  
		int i = 1;
		while (cookies)
		{
			//printf("%s\n", cookies->data);
			Cookies.append(cookies->data);
			cookies = cookies->next;
			i++;
		}
	}

	if (curl_header) {
		curl_slist_free_all((curl_slist*)curl_header);
		curl_header = NULL;
	}
	curl_easy_cleanup(curl);
	return RESPONSE_CODE;
};
int WebClient::HttpGet(const std::string& strUrl, std::string* strResponse, int nTimeout) {
	CURL* curl = Init(strUrl, strResponse, nTimeout);
	if (!curl) {
		return CURLE_FAILED_INIT;
	}
	CURLcode code = curl_easy_perform(curl);
	return CleanUp(curl, code);
};
int WebClient::HttpPost(const std::string& url, const std::string& data, std::string* respone, int _timeout) {

	CURL* curl = Init(url, respone, _timeout);
	if (!curl) {
		return CURLE_FAILED_INIT;
	}
	curl_easy_setopt(curl, CURLOPT_POST, true);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	CURLcode code = curl_easy_perform(curl);
	return CleanUp(curl, code);
};
int WebClient::UploadFile(const std::string& url, const std::string& filename, const std::string& field, std::string* respone, const std::function<void(long long dltotal, long long dlnow)>& progressCallback, int _timeout) {

	CURL* curl = Init(url, respone, _timeout);
	if (!curl) {
		return CURLE_FAILED_INIT;
	}
	struct curl_httppost* formpost = 0;
	struct curl_httppost* lastptr = 0;
	curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, field.c_str(), CURLFORM_FILE, filename.c_str(), CURLFORM_END);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	StructCallback struct_cb;
	if (progressCallback) {
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);//接受上传下载进度
		//curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressCallback);//将函数回调函数设置传入指针
		//curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, g_curl_progress_callback);//进度回调
		struct_cb.progressCallback = progressCallback;
		struct_cb.lastUpdate = ::GetTickCount64();
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &struct_cb);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, g_curl_progress_callback);//进度回调
	}

	CURLcode  code = curl_easy_perform(curl);
	if (formpost) {
		curl_formfree(formpost);
	}

	return CleanUp(curl, code);
};

//multipart/form-data方式,适用于文件上传，数据被分为多个部分，每个部分都有自己的标头。
int WebClient::SubmitForm(const std::string& strUrl, const std::vector<PostForm::Field>& fieldValues, std::string* respone, int nTimeout) {

	AddHeader("Content-Type", "multipart/form-data");
	CURL* curl = Init(strUrl, respone, nTimeout);
	if (!curl) {
		return CURLE_FAILED_INIT;
	}
	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	// 设置表头，表头内容可能不同
	for (auto& item : fieldValues) {
		if (item.FieldType == PostForm::FieldType::File) {
			curl_formadd(&formpost, &lastptr,
				CURLFORM_COPYNAME, item.FieldName.c_str(),
				CURLFORM_FILE, item.FileName.c_str(),//此处不安全 需要使用unicode才行
				CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		}
		if (item.FieldType == PostForm::FieldType::Text) {
			curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, item.FieldName.c_str(), CURLFORM_COPYCONTENTS, item.FieldValue.c_str(), CURLFORM_END);
		}
	}
	// 设置表单参数
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	CURLcode code = curl_easy_perform(curl);

	if (formpost) {
		curl_formfree(formpost);
	}
	return CleanUp(curl, code);

};
int WebClient::DownloadFile(const std::string& url, const std::wstring& _filename, const std::function<void(long long dltotal, long long dlnow)>& progressCallback, int nTimeout) {
	std::string resp;
	CURL* curl = Init(url, &resp, nTimeout);
	if (!curl) {
		return CURLE_FAILED_INIT;
	}
	File::Delete(_filename);

	std::ofstream ofs(_filename, std::ios::app | std::ios::binary);
	this->content.tag = &ofs;
	this->content.type = 1;

	StructCallback struct_cb;
	if (progressCallback) {
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);//接受上传下载进度
		//curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressCallback);//将函数回调函数设置传入指针
		//curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, g_curl_progress_callback);//进度回调
		struct_cb.progressCallback = progressCallback;
		struct_cb.lastUpdate = ::GetTickCount64();
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &struct_cb);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, g_curl_progress_callback);//进度回调
	}

	CURLcode  code = curl_easy_perform(curl);
	return CleanUp(curl, code);
};
int WebClient::FtpDownLoad(const std::string& strUrl, const std::string& user, const std::string& pwd, const std::wstring& outFileName, int nTimeout) {

	std::string resp;
	CURL* curl = Init(strUrl, &resp, nTimeout);
	if (!curl) {
		return CURLE_FAILED_INIT;
	}

	std::ofstream ofs(outFileName, std::ios::app | std::ios::binary);
	this->content.tag = &ofs;
	this->content.type = 1;

	if (!user.empty() && !pwd.empty()) {
		curl_easy_setopt(curl, CURLOPT_USERPWD, (user + ":" + pwd).c_str());
	}
	else {
		curl_easy_setopt(curl, CURLOPT_USERPWD, "");
	}
	CURLcode code = curl_easy_perform(curl);
	return CleanUp(curl, code);
}
void WebClient::AddHeader(const std::string& key, const std::string& value)
{
	Header.insert(std::pair<std::string, std::string>(key, value));
}
void WebClient::RemoveHeader(const std::string& key)
{
	Header.erase(key);
};

#endif