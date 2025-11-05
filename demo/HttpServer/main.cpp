#include "httplib.h"
#include "WebClient.h"
#include "Log.h"
#include "Util.h"
#include "WinTool.h"
#include "ConfigIni.h"
#include "Text.h"
#include "FileSystem.h"
#include "JsonValue.h"
#include "main.h"
using namespace httplib;
ConfigIni cfg(Path::StartPath() + "/users.ini");


void ResponseEnCode(const httplib::Request& req, httplib::Response& res, const std::string& data, const std::string& contentType = "text/plain", bool log = true) {
	u8String timeStr = req.get_header_value("datetime");
	u8String user = req.get_header_value("user");

	if (timeStr.empty() || user.empty()) {
		res.status = 500;
	}
	u8String winver = req.get_header_value("winver");
	u8String computerName = req.get_header_value("computerName");
	u8String loginUser = req.get_header_value("loginUser");

	cfg.SetSection(user);
	cfg.WriteString("winver", winver);
	cfg.WriteString("computerName", computerName);
	cfg.WriteString("loginUser", loginUser);
	cfg.WriteString("ip", req.remote_addr);

	if (log) {
		printf("\n");
		Log::Info(L"登录用户名:%s 计算机名称:%s Windows版本:%s", loginUser.c_str(), computerName.c_str(), winver.c_str());
		Log::Info(L"用户ID:[%s] 接口:%s", user.c_str(), req.path.c_str());
	}

	u8String password = Util::MD5FromString(timeStr + user);
	u8String repData = Util::XOR::EnCode(data, password);
	res.set_content(repData, contentType);
}

void NewFunction(const httplib::Request& req, httplib::Response& res, bool log)
{
	u8String user = req.get_param_value("user");
	cfg.SetSection(user);

	int count = cfg.ReadInt("count", 0);
	int max = cfg.ReadInt("max", 2);
	//新用户
	u8String date = cfg.ReadString("reg_date", "");
	if (date.empty()) {
		cfg.WriteString("reg_date", Time::Now().ToString());
		cfg.WriteString("max", std::to_string(max));
		cfg.WriteString("count", std::to_string(count));
		cfg.WriteString("cmd", "");
		cfg.WriteString("msg", "");
	}
	if (count > max) {
		res.status = 500;
	}
	else {
		res.status = 200;
	}

	//日期限制
	//Time t(date);
	//auto diff = Time::Now() - t;
	////每个计算机注册账号之后只有两天的使用时间
	//if (diff > (2 * 24 * 60 * 60)) {
	//	res.status = 500;
	//	return;
	//}

	Json::Value json;
	json["req_time"] = req.get_param_value("time");
	json["time"] = Time::Now().ToString();
	json["user"] = user;
	json["max"] = max;
	json["count"] = count;
	json["msg"] = cfg.ReadString("msg", "");
	json["cmd"] = cfg.ReadString("cmd", "");

	ResponseEnCode(req, res, json.toString(), "text/plain", log);
	if (log) {
		Log::Info(L"[%s]IP:%s 返回code:%d,最大次数:%d 已刷次数:%d", user.c_str(), req.remote_addr.c_str(), res.status, max, count);
	}
}
int main(int count, const char** args)
{
	Log::WriteFile = true;
	Server svr;

	svr.Get("/login", [&](const Request& req, httplib::Response& res) {
		NewFunction(req, res, true);
		});
	//获取版本
	svr.Get("/ver", [&](const Request& req, httplib::Response& res) {
		NewFunction(req, res, false);
		});

	svr.Get("/", [&](const Request& req, httplib::Response& res) {
		res.set_content("hello", "text/plain");
		});

	//向服务器报告使用次数
	svr.Get("/reportFlashROW", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		cfg.SetSection(user);
		int count = cfg.ReadInt("count", 0);
		count++;
		cfg.WriteString("count", std::to_string(count));
		ResponseEnCode(req, res, std::to_string(count), "text/plain");
		Log::Info(L"[%s]刚刚刷了国际版:%d", user.c_str(), count);
		});

	//向服务器报告使用次数
	svr.Get("/reportUnlock", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		u8String mode = req.get_param_value("mode");

		cfg.SetSection(user);

		int unlock = cfg.ReadInt("unlock", 0);
		unlock++;
		cfg.WriteString("unlock", std::to_string(unlock));
		ResponseEnCode(req, res, std::to_string(unlock), "text/plain");
		Log::Info(L"[%s][%s]已请求解锁:%d", user.c_str(), mode.c_str(), unlock);
		});

	svr.Get("/reportDownload", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		u8String btn = req.get_param_value("btn");
		cfg.SetSection(user);

		int count = cfg.ReadInt("count", 0);
		count++;
		cfg.WriteString("count", std::to_string(count));
		ResponseEnCode(req, res, std::to_string(count), "text/plain");
		Log::Info(L"[%s]已点击下载按钮:[%s]", user.c_str(), btn.c_str());
		});

	svr.Get("/reportButton", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		u8String btn = req.get_param_value("btn");
		cfg.SetSection(user);

		int count = cfg.ReadInt("count", 0);
		count++;
		cfg.WriteString("count", std::to_string(count));
		ResponseEnCode(req, res, std::to_string(count), "text/plain");
		Log::Info(L"[%s]已点击按钮:[%s]", user.c_str(), btn.c_str());
		});

	//重置刷机次数
	svr.Get("/reset", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		cfg.SetSection(user);

		int count = 0;
		cfg.WriteString("count", std::to_string(count));
		ResponseEnCode(req, res, std::to_string(count), "text/plain");
		Log::Info(L"[%s]刷机次数已重置:%d", user.c_str(), count);
		});

	svr.Get("/admin/users.ini", [&](const Request& req, httplib::Response& res) {
		std::string data;
		File::ReadFile(Path::StartPath() + "/users.ini", &data);
		res.set_content(data, "text/plain;charset=utf-8");
		});

	svr.Get("/admin/setcount", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		u8String count = req.get_param_value("count");
		cfg.SetSection(user);

		cfg.WriteString("count", count);
		res.set_content(u8String(L"当前使用次数修改完毕") + count, "text/plain;charset=utf-8");
		Log::Info(L"[%s]修改用户使用次数:%s", user.c_str(), count.c_str());
		});

	svr.Get("/admin/setmax", [&](const Request& req, httplib::Response& res) {
		u8String user = req.get_param_value("user");
		u8String max = req.get_param_value("max");
		cfg.SetSection(user);

		cfg.WriteString("max", max);
		res.set_content(u8String(L"最大使用次数修改完毕") + max, "text/plain;charset=utf-8");
		Log::Info(L"[%s]修改用户最大使用次数:%s", user.c_str(), max.c_str());
		});

	Log::Info(L"服务已启动!");
	svr.listen("0.0.0.0", 80);
}

