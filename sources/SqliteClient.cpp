#include "SqliteClient.h"

#include "sqlite/sqlite3.h"

// 安全转义函数 - 防止 SQL 注入
static std::string EscapeSQLString(const std::string& input) {
	std::string result;
	for (char c : input) {
		if (c == '\'') {
			result += "''"; // SQLite 使用双单引号转义
		}
		else {
			result += c;
		}
	}
	return result;
}

SqliteClient::SqliteClient(const std::string& dbPath)
{
	this->dbPath = dbPath;
}

bool SqliteClient::OpenConn()
{
	return sqlite3_open(dbPath.c_str(), (sqlite3**)&db) == SQLITE_OK;
}

void SqliteClient::CloseConn()
{
	if (db) {
		sqlite3_close((sqlite3*)db);
		db = nullptr;
	}
}

bool SqliteClient::ExecuteQuery(const std::string& sql, std::string& result)
{
	// 使用成员变量连接，避免频繁打开/关闭
	if (!db) {
		result = "[]";
		return false;
	}

	sqlite3_stmt* stmt = nullptr;
	bool ok = false;

	do {
		if (sqlite3_prepare_v2((sqlite3*)db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
			break;
		}

		int colCount = sqlite3_column_count(stmt);
		if (colCount == 0) {
			result = "[]";
			ok = true;
			break;
		}

		// 获取列名
		std::vector<std::string> fields;
		for (int i = 0; i < colCount; i++) {
			const char* name = sqlite3_column_name(stmt, i);
			fields.push_back(name ? name : "");
		}

		// 构建JSON结果
		result = "[";
		size_t rowCount = 0;

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			rowCount++;
			result += "{";

			for (int i = 0; i < colCount; i++) {
				result += "\"" + fields[i] + "\":";

				const unsigned char* text = sqlite3_column_text(stmt, i);
				if (text) {
					result += "\"" + std::string((const char*)text) + "\"";
				}
				else {
					result += "\"\"";
				}

				if (i < colCount - 1) {
					result += ",";
				}
			}

			result += "},";
		}

		if (rowCount > 0) {
			result.pop_back(); // 移除最后的逗号
		}
		result += "]";

		if (rowCount == 0) {
			result = "[]";
		}

		ok = true;

	} while (false);

	if (stmt) {
		sqlite3_finalize(stmt);
	}

	return ok;
}

size_t SqliteClient::ExecuteNoQuery(const std::string& sql)
{
	// 使用成员变量连接，避免频繁打开/关闭
	if (!db) {
		return 0;
	}

	char* errMsg = nullptr;
	int result = sqlite3_exec((sqlite3*)db, sql.c_str(), nullptr, nullptr, &errMsg);

	size_t affectedRows = 0;
	if (result == SQLITE_OK) {
		affectedRows = sqlite3_changes((sqlite3*)db);
	}
	else if (errMsg) {
		sqlite3_free(errMsg);
	}

	return affectedRows;
}

size_t SqliteClient::Insert(const std::string& tableName, const Json::Value& jv)
{
	// 边界检查：表名和字段不能为空
	if (tableName.empty() || jv.getMemberNames().empty()) {
		return 0;
	}

	std::string keys = "(";
	std::string values = "(";

	for (const auto& key : jv.getMemberNames()) {
		keys += key + ",";

		if (jv[key].isNumeric()) {
			values += jv[key].toString() + ",";
		}
		else if (jv[key].isString()) {
			// 使用转义函数防止 SQL 注入
			values += "'" + EscapeSQLString(jv[key].asString()) + "',";
		}
		else {
			// 使用转义函数防止 SQL 注入
			values += "'" + EscapeSQLString(jv[key].toString()) + "',";
		}
	}

	keys.pop_back(); // 移除最后的逗号
	values.pop_back();
	keys += ")";
	values += ")";

	std::string sql = "INSERT INTO " + tableName + " " + keys + " VALUES " + values;
	return ExecuteNoQuery(sql);
}

size_t SqliteClient::Update(const std::string& tableName, const Json::Value& jv, const std::string& whereText)
{
	// 边界检查：表名不能为空，WHERE 条件不能为空，字段不能为空
	if (tableName.empty() || whereText.empty() || jv.getMemberNames().empty()) {
		return 0;
	}

	std::string sql = "UPDATE " + tableName + " SET ";

	for (const auto& key : jv.getMemberNames()) {
		sql += key + "=";

		if (jv[key].isNumeric()) {
			sql += jv[key].toString();
		}
		else if (jv[key].isString()) {
			// 使用转义函数防止 SQL 注入
			sql += "'" + EscapeSQLString(jv[key].asString()) + "'";
		}
		else {
			// 使用转义函数防止 SQL 注入
			sql += "'" + EscapeSQLString(jv[key].toString()) + "'";
		}

		sql += ",";
	}

	sql.pop_back(); // 移除最后的逗号
	sql += " " + whereText;

	return ExecuteNoQuery(sql);
}

bool SqliteClient::ExecuteQuery(const std::string& sql, Json::Value& result)
{
	std::string str;
	bool ret = ExecuteQuery(sql, str);
	result = JsonValue(str);
	return ret;
}

SqliteClient::~SqliteClient()
{
	CloseConn();
}