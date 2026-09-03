#pragma once
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "HttpTransport.h"

class HttpRequest {
public:
	std::string url;
	std::map<std::string, std::string> Header;

	HttpRequest() {}
	HttpRequest(const std::string& url) : url(url) {}
	void AddHeader(const std::string& key, const std::string& value) { Header[key] = value; }
	void AddHreader(const std::string& key, const std::string& value) { AddHeader(key, value); }
};

class Cookie {
public:
	std::string domain;
	bool includeSubDomain = false;
	std::string path = "/";
	bool secure = false;
	long long expires = 0;
	bool httpOnly = false;
	std::string name;
	std::string value;
	Cookie() = default;
	Cookie(const std::string& headerLine, const std::string& defaultHost = "", const std::string& defaultPath = "/");
	std::string ToString()const;
	bool MatchesHost(const std::string& host)const;
};

// HTTP 客户端，自动管理 cookie 和重定向
class WebClient {
	std::vector<Cookie> cookies;
	mutable std::mutex cookiesMutex;
	std::string GetHost(const std::string& strUrl)const;
	void ApplyCookies(const std::string& url, HttpTransport& http);
	void SaveCookies(const std::string& url, const Text::String& cookieText);
public:
	std::string Proxy;
	WebClient();
	virtual ~WebClient();
	bool SetCookie(const Cookie& cookie);
	Cookie GetCookie(const std::string& host);
	int HttpGet(const HttpRequest& request, std::string* response = NULL, int nTimeout = 60);
	int HttpPost(const HttpRequest& request, const std::string& data = "", std::string* response = NULL, int nTimeout = 60);
	int HttpDelete(const HttpRequest& request, std::string* response = NULL, int nTimeout = 60);
	int DownloadFile(const HttpRequest& request, const std::wstring& filename, const std::function<void(long long dltotal, long long dlnow)>& progressCallback = NULL, int nTimeout = 99999);
	int SubmitForm(const HttpRequest& request, const std::vector<PostForm::Field>& fieldValues, std::string* response = NULL, int nTimeout = 99999);
};
