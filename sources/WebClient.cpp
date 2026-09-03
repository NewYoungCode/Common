#include "WebClient.h"

#include <cctype>
#include <cstdlib>
#include <ctime>
#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#include "curl/curl.h"

namespace {

	std::string Trim(const std::string& value)
	{
		size_t begin = 0;
		while (begin < value.size() && std::isspace((unsigned char)value[begin])) {
			begin++;
		}

		size_t end = value.size();
		while (end > begin && std::isspace((unsigned char)value[end - 1])) {
			end--;
		}
		return value.substr(begin, end - begin);
	}

	std::string ToLower(std::string value)
	{
		for (auto& ch : value) {
			ch = (char)std::tolower((unsigned char)ch);
		}
		return value;
	}

	bool StartsWith(const std::string& value, const std::string& prefix)
	{
		return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
	}

	bool StartsWithNoCase(const std::string& value, const std::string& prefix)
	{
		return value.size() >= prefix.size() && ToLower(value.substr(0, prefix.size())) == ToLower(prefix);
	}

	bool EndsWith(const std::string& value, const std::string& suffix)
	{
		return value.size() >= suffix.size() && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	std::vector<std::string> Split(const std::string& value, char delimiter)
	{
		std::vector<std::string> fields;
		size_t begin = 0;
		while (begin <= value.size()) {
			size_t end = value.find(delimiter, begin);
			if (end == std::string::npos) {
				fields.push_back(value.substr(begin));
				break;
			}
			fields.push_back(value.substr(begin, end - begin));
			begin = end + 1;
		}
		return fields;
	}

	std::string NormalizeCookieHost(const std::string& host)
	{
		std::string value = Trim(host);
		const std::string httpOnlyPrefix = "#HttpOnly_";
		if (StartsWith(value, httpOnlyPrefix)) {
			value = value.substr(httpOnlyPrefix.size());
		}
		return ToLower(value);
	}

	std::vector<std::string> SplitCookieList(const std::string& cookieText)
	{
		std::vector<std::string> items;
		std::string trimmed = Trim(cookieText);
		if (trimmed.empty()) {
			return items;
		}

		if (cookieText.find('\n') != std::string::npos) {
			size_t begin = 0;
			while (begin <= cookieText.size()) {
				size_t end = cookieText.find('\n', begin);
				if (end == std::string::npos) {
					items.push_back(cookieText.substr(begin));
					break;
				}
				items.push_back(cookieText.substr(begin, end - begin));
				begin = end + 1;
			}
			return items;
		}

		items.push_back(trimmed);
		return items;
	}

	bool ParseCookiePair(const std::string& text, std::string* key, std::string* value)
	{
		size_t pos = text.find('=');
		if (pos == std::string::npos) {
			return false;
		}

		std::string parsedKey = Trim(text.substr(0, pos));
		if (parsedKey.empty()) {
			return false;
		}

		if (key) {
			*key = parsedKey;
		}
		if (value) {
			*value = Trim(text.substr(pos + 1));
		}
		return true;
	}

	bool ParseCookieBool(const std::string& value)
	{
		std::string text = ToLower(Trim(value));
		return text == "1" || text == "true" || text == "yes";
	}

	void ApplyHeaders(const HttpRequest& request, HttpTransport& http)
	{
		for (const auto& item : request.Header) {
			http.AddHeader(item.first, item.second);
		}
	}

	bool CookieExpired(const Cookie& cookie)
	{
		return cookie.expires > 0 && cookie.expires <= (long long)std::time(NULL);
	}

	bool SameCookieKey(const Cookie& left, const Cookie& right)
	{
		return NormalizeCookieHost(left.domain) == NormalizeCookieHost(right.domain) &&
			left.path == right.path &&
			left.name == right.name;
	}

	std::string GetUrlPath(const std::string& strUrl)
	{
		std::string url = Trim(strUrl);
		size_t begin = url.find("://");
		begin = begin == std::string::npos ? 0 : begin + 3;

		size_t path = url.find('/', begin);
		if (path == std::string::npos) {
			return "/";
		}

		size_t end = url.find_first_of("?#", path);
		std::string value = url.substr(path, end == std::string::npos ? std::string::npos : end - path);
		return value.empty() ? "/" : value;
	}

	std::string GetDefaultCookiePath(const std::string& strUrl)
	{
		std::string path = GetUrlPath(strUrl);
		if (path.empty() || path[0] != '/') {
			return "/";
		}
		if (path == "/") {
			return "/";
		}

		size_t pos = path.rfind('/');
		if (pos == std::string::npos || pos == 0) {
			return "/";
		}
		return path.substr(0, pos);
	}

	bool IsSecureUrl(const std::string& strUrl)
	{
		std::string url = Trim(strUrl);
		size_t schemeEnd = url.find("://");
		return schemeEnd != std::string::npos && ToLower(url.substr(0, schemeEnd)) == "https";
	}

	bool CookiePathMatches(const std::string& requestPath, const std::string& cookiePath)
	{
		std::string storedPath = cookiePath.empty() ? "/" : cookiePath;
		std::string path = requestPath.empty() ? "/" : requestPath;
		if (storedPath == "/" || path == storedPath) {
			return true;
		}
		if (!StartsWith(path, storedPath)) {
			return false;
		}
		return storedPath.back() == '/' || (path.size() > storedPath.size() && path[storedPath.size()] == '/');
	}

	bool CookieMoreSpecific(const Cookie& candidate, const Cookie& current)
	{
		if (candidate.path.size() != current.path.size()) {
			return candidate.path.size() > current.path.size();
		}

		std::string candidateDomain = NormalizeCookieHost(candidate.domain);
		std::string currentDomain = NormalizeCookieHost(current.domain);
		if (!candidateDomain.empty() && candidateDomain[0] == '.') {
			candidateDomain.erase(0, 1);
		}
		if (!currentDomain.empty() && currentDomain[0] == '.') {
			currentDomain.erase(0, 1);
		}
		if (candidateDomain.size() != currentDomain.size()) {
			return candidateDomain.size() > currentDomain.size();
		}

		bool candidateHostOnly = !candidate.includeSubDomain;
		bool currentHostOnly = !current.includeSubDomain;
		return candidateHostOnly && !currentHostOnly;
	}

	void NormalizeCookie(Cookie& cookie)
	{
		cookie.domain = Trim(cookie.domain);
		const std::string httpOnlyPrefix = "#HttpOnly_";
		if (StartsWith(cookie.domain, httpOnlyPrefix)) {
			cookie.httpOnly = true;
			cookie.domain = cookie.domain.substr(httpOnlyPrefix.size());
		}
		cookie.domain = ToLower(cookie.domain);
		if (cookie.path.empty()) {
			cookie.path = "/";
		}
		else if (cookie.path[0] != '/') {
			cookie.path = "/" + cookie.path;
		}
		if (!cookie.domain.empty() && cookie.domain[0] == '.') {
			cookie.includeSubDomain = true;
		}
		else if (cookie.includeSubDomain && !cookie.domain.empty()) {
			cookie.domain = "." + cookie.domain;
		}
	}

	void RemoveCookieLocked(std::vector<Cookie>& cookies, const Cookie& cookie)
	{
		for (auto it = cookies.begin(); it != cookies.end(); ) {
			if (SameCookieKey(*it, cookie)) {
				it = cookies.erase(it);
			}
			else {
				++it;
			}
		}
	}

	long long ParseCookieExpires(const std::string& expiresText)
	{
		time_t value = curl_getdate(expiresText.c_str(), nullptr);
		return value < 0 ? 0 : (long long)value;
	}
}

Cookie::Cookie(const std::string& headerLine, const std::string& defaultHost, const std::string& defaultPath)
{
	std::string cookieText = Trim(headerLine);
	if (StartsWithNoCase(cookieText, "Set-Cookie:")) {
		cookieText = Trim(cookieText.substr(sizeof("Set-Cookie:") - 1));
	}
	if (cookieText.empty()) {
		return;
	}

	auto fields = Split(cookieText, ';');
	if (fields.empty()) {
		return;
	}

	std::string name;
	std::string fieldValue;
	if (!ParseCookiePair(fields[0], &name, &fieldValue)) {
		return;
	}

	domain = defaultHost;
	path = defaultPath.empty() ? "/" : defaultPath;
	this->name = name;
	value = fieldValue;

	bool hasDomainAttribute = false;
	bool hasMaxAge = false;
	for (size_t i = 1; i < fields.size(); ++i) {
		std::string attribute = Trim(fields[i]);
		if (attribute.empty()) {
			continue;
		}

		std::string attributeName;
		std::string attributeValue;
		if (!ParseCookiePair(attribute, &attributeName, &attributeValue)) {
			std::string flagName = ToLower(attribute);
			if (flagName == "secure") {
				secure = true;
			}
			else if (flagName == "httponly") {
				httpOnly = true;
			}
			continue;
		}

		std::string flagName = ToLower(attributeName);
		if (flagName == "domain") {
			domain = attributeValue.empty() ? defaultHost : attributeValue;
			includeSubDomain = true;
			hasDomainAttribute = true;
		}
		else if (flagName == "path") {
			path = attributeValue.empty() ? "/" : attributeValue;
		}
		else if (flagName == "expires" && !hasMaxAge) {
			expires = ParseCookieExpires(attributeValue);
		}
		else if (flagName == "max-age") {
			hasMaxAge = true;
			long long seconds = std::atoll(attributeValue.c_str());
			expires = seconds <= 0 ? (long long)std::time(NULL) - 1 : (long long)std::time(NULL) + seconds;
		}
		else if (flagName == "__cookie_include_sub_domain") {
			includeSubDomain = ParseCookieBool(attributeValue);
		}
		else if (flagName == "__cookie_expires") {
			expires = std::atoll(attributeValue.c_str());
		}
	}

	if (!hasDomainAttribute) {
		domain = defaultHost;
		includeSubDomain = false;
	}
}

std::string Cookie::ToString()const
{
	std::string cookieText;
	auto AddPart = [&](const std::string& part) {
		if (part.empty()) {
			return;
		}
		if (!cookieText.empty()) {
			cookieText += "; ";
		}
		cookieText += part;
		};

	std::string primaryName = name;
	std::string primaryValue = value;

	if (primaryName.empty()) {
		return "";
	}
	AddPart(primaryName + "=" + primaryValue);

	if (!domain.empty()) {
		AddPart("Domain=" + domain);
	}
	AddPart("__cookie_include_sub_domain=" + std::string(includeSubDomain ? "1" : "0"));
	AddPart("Path=" + (path.empty() ? "/" : path));
	if (secure) {
		AddPart("Secure");
	}
	AddPart("__cookie_expires=" + std::to_string(expires));
	if (httpOnly) {
		AddPart("HttpOnly");
	}
	return cookieText;
}

bool Cookie::MatchesHost(const std::string& requestHost)const
{
	std::string host = ToLower(Trim(requestHost));
	std::string cookieDomain = NormalizeCookieHost(domain);
	if (cookieDomain.empty()) {
		return true;
	}

	if (host == cookieDomain) {
		return true;
	}

	if (cookieDomain[0] == '.') {
		std::string rootDomain = cookieDomain.substr(1);
		return host == rootDomain || EndsWith(host, cookieDomain);
	}

	if (includeSubDomain) {
		return EndsWith(host, "." + cookieDomain);
	}
	return false;
}

WebClient::WebClient()
{
}

WebClient::~WebClient()
{
}

std::string WebClient::GetHost(const std::string& strUrl)const
{
	std::string url = Trim(strUrl);
	size_t begin = url.find("://");
	begin = begin == std::string::npos ? 0 : begin + 3;

	size_t end = url.find_first_of("/?#", begin);
	std::string host = url.substr(begin, end == std::string::npos ? std::string::npos : end - begin);

	size_t userInfo = host.rfind('@');
	if (userInfo != std::string::npos) {
		host = host.substr(userInfo + 1);
	}

	if (!host.empty() && host[0] == '[') {
		size_t ipv6End = host.find(']');
		if (ipv6End != std::string::npos) {
			return ToLower(host.substr(0, ipv6End + 1));
		}
	}

	size_t port = host.find(':');
	if (port != std::string::npos) {
		host = host.substr(0, port);
	}
	return ToLower(host);
}

void WebClient::ApplyCookies(const std::string& url, HttpTransport& http)
{
	std::string host = GetHost(url);
	std::string path = GetUrlPath(url);
	bool secureRequest = IsSecureUrl(url);
	std::map<std::string, Cookie> selectedCookies;

	{
		std::lock_guard<std::mutex> lock(cookiesMutex);
		for (auto it = cookies.begin(); it != cookies.end(); ) {
			if (CookieExpired(*it)) {
				it = cookies.erase(it);
				continue;
			}

			if (!it->name.empty() &&
				it->MatchesHost(host) &&
				CookiePathMatches(path, it->path) &&
				(!it->secure || secureRequest)) {
				auto existing = selectedCookies.find(it->name);
				if (existing == selectedCookies.end() || CookieMoreSpecific(*it, existing->second)) {
					selectedCookies[it->name] = *it;
				}
			}
			++it;
		}
	}

	for (const auto& item : selectedCookies) {
		http.AddCookie(item.second.name, item.second.value);
	}
}

void WebClient::SaveCookies(const std::string& url, const Text::String& cookieText)
{
	std::string host = GetHost(url);
	std::string defaultPath = GetDefaultCookiePath(url);
	for (auto& item : SplitCookieList(cookieText)) {
		item = Trim(item);
		if (item.empty()) {
			continue;
		}

		Cookie cookie(item, host, defaultPath);

		if (!cookie.name.empty()) {
			SetCookie(cookie);
		}
	}
}

bool WebClient::SetCookie(const Cookie& cookie)
{
	Cookie newCookie = cookie;
	NormalizeCookie(newCookie);
	if (newCookie.name.empty()) {
		return false;
	}

	std::lock_guard<std::mutex> lock(cookiesMutex);
	if (CookieExpired(newCookie)) {
		RemoveCookieLocked(cookies, newCookie);
		return false;
	}

	for (auto& stored : cookies) {
		if (SameCookieKey(stored, newCookie)) {
			stored = newCookie;
			return true;
		}
	}
	cookies.push_back(newCookie);
	return true;
}

Cookie WebClient::GetCookie(const std::string& host)
{
	std::string requestHost = GetHost(host);
	Cookie result;
	result.domain = requestHost;
	bool found = false;

	std::lock_guard<std::mutex> lock(cookiesMutex);
	for (auto it = cookies.begin(); it != cookies.end(); ) {
		if (CookieExpired(*it)) {
			it = cookies.erase(it);
			continue;
		}

		if (it->MatchesHost(requestHost) && !it->name.empty()) {
			if (!found || CookieMoreSpecific(*it, result)) {
				result = *it;
				found = true;
			}
		}
		++it;
	}
	return result;
}

int WebClient::HttpGet(const HttpRequest& request, std::string* response, int nTimeout)
{
	std::string tempResponse;
	HttpTransport http;
	http.Proxy = Proxy;
	ApplyHeaders(request, http);

	ApplyCookies(request.url, http);

	int code = http.HttpGet(request.url, response ? response : &tempResponse, nTimeout);
	SaveCookies(request.url, http.GetCookie());
	return code;
}

int WebClient::HttpPost(const HttpRequest& request, const std::string& data, std::string* response, int nTimeout)
{
	std::string tempResponse;
	HttpTransport http;
	http.Proxy = Proxy;
	ApplyHeaders(request, http);

	ApplyCookies(request.url, http);

	int code = http.HttpPost(request.url, data, response ? response : &tempResponse, nTimeout);
	SaveCookies(request.url, http.GetCookie());
	return code;
}

int WebClient::HttpDelete(const HttpRequest& request, std::string* response, int nTimeout)
{
	std::string tempResponse;
	HttpTransport http;
	http.Proxy = Proxy;
	ApplyHeaders(request, http);

	ApplyCookies(request.url, http);

	int code = http.HttpDelete(request.url, response ? response : &tempResponse, nTimeout);
	SaveCookies(request.url, http.GetCookie());
	return code;
}

int WebClient::DownloadFile(const HttpRequest& request, const std::wstring& filename, const std::function<void(long long dltotal, long long dlnow)>& progressCallback, int nTimeout)
{
	HttpTransport http;
	http.Proxy = Proxy;
	ApplyHeaders(request, http);

	ApplyCookies(request.url, http);

	int code = http.DownloadFile(request.url, filename, progressCallback, nTimeout);
	SaveCookies(request.url, http.GetCookie());
	return code;
}

int WebClient::SubmitForm(const HttpRequest& request, const std::vector<PostForm::Field>& fieldValues, std::string* response, int nTimeout)
{
	std::string tempResponse;
	HttpTransport http;
	http.Proxy = Proxy;
	ApplyHeaders(request, http);

	ApplyCookies(request.url, http);

	int code = http.SubmitForm(request.url, fieldValues, response ? response : &tempResponse, nTimeout);
	SaveCookies(request.url, http.GetCookie());
	return code;
}
