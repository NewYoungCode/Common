#include "IniConfig.h"
#include "SimpleIni.h"
#include "FileSystem.h"

#include <sstream>
#include <algorithm>

// ─── 内部辅助：加载文件，操作，保存文件 ──────────────────────────

namespace {

	//// 每次操作新建 CSimpleIniA，加载文件，执行 op，再保存回去
	//// op 签名：void(CSimpleIniA&)
	//template<typename Op>
	//static bool WithIni(const Text::String& filename, Op op) {
	//	std::wstring wfilename = filename.unicode();

	//	CSimpleIniA ini;
	//	ini.SetUnicode(true);       // UTF-8
	//	ini.SetMultiKey(false); // 不允许重复 key

	//	SI_Error rc = ini.LoadFile(wfilename.c_str());
	//	// 文件不存在时 LoadFile 返回 SI_FILE，允许继续（相当于空文件）
	//	if (rc < 0 && rc != SI_FILE) return false;

	//	op(ini);

	//	return ini.SaveFile(wfilename.c_str()) >= 0;
	//}

	//// 只读操作，不需要保存
	//template<typename Op>
	//static void WithIniReadOnly(const Text::String& filename, Op op) {
	//	std::wstring wfilename = filename.unicode();

	//	CSimpleIniA ini;
	//	ini.SetUnicode(true);
	//	ini.SetMultiKey(false);
	//	if (ini.LoadFile(wfilename.c_str()) < 0) return;
	//	op(ini);
	//}

} // namespace

// ─── IniConfig 实现 ──────────────────────────────────────────────

bool IniConfig::WithIniReadOnly(const std::function<void(void*)>& op)
{
	std::lock_guard<std::mutex> lock(m_mtx);

	CSimpleIniA ini;
	ini.SetUnicode(true);
	ini.SetMultiKey(false);
	if (ini.LoadFile(m_filename.c_str()) < 0) return false;
	op((void*)&ini);

	return true;
}
bool IniConfig::WithIni(const std::function<void(void*)>& op)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	CSimpleIniA ini;
	ini.SetUnicode(true);       // UTF-8
	ini.SetMultiKey(false); // 不允许重复 key
	SI_Error rc = ini.LoadFile(m_filename.c_str());
	// 文件不存在时 LoadFile 返回 SI_FILE，允许继续（相当于空文件）
	if (rc < 0 && rc != SI_FILE) return false;
	op((void*)&ini);
	return ini.SaveFile(m_filename.c_str()) >= 0;
}

IniConfig::IniConfig(const Text::String& filename)
	: m_filename(filename.unicode())
{
}

// ─── Section / Key 管理 ──────────────────────────────────────────

std::vector<Text::String> IniConfig::GetSections() {
	std::vector<Text::String> result;
	WithIniReadOnly([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;

		CSimpleIniA::TNamesDepend sections;
		ini.GetAllSections(sections);
		for (const auto& entry : sections)
			result.emplace_back(entry.pItem);
		});
	return result;
}

void IniConfig::DeleteSection(const Text::String& section) {
	WithIni([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		ini.Delete(section.c_str(), nullptr); // key=nullptr 表示删除整个 section
		});
}

std::vector<Text::String> IniConfig::GetKeys(const Text::String& section) {
	std::vector<Text::String> result;
	WithIniReadOnly([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		CSimpleIniA::TNamesDepend keys;
		if (ini.GetAllKeys(section.c_str(), keys)) {
			for (const auto& entry : keys)
				result.emplace_back(entry.pItem);
		}
		});
	return result;
}

void IniConfig::DeleteKey(const Text::String& section, const Text::String& key) {
	WithIni([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		ini.Delete(section.c_str(), key.c_str());
		});
}

// ─── Read ────────────────────────────────────────────────────────

Text::String IniConfig::ReadString(const Text::String& section, const Text::String& key,
	const Text::String& defaultValue) {
	Text::String result = defaultValue;
	WithIniReadOnly([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		const char* val = ini.GetValue(section.c_str(), key.c_str(), nullptr);
		if (val) result = val;
		});
	return result;
}

float IniConfig::ReadFloat(const Text::String& section, const Text::String& key,
	float defaultValue) {
	float result = defaultValue;
	WithIniReadOnly([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		const char* val = ini.GetValue(section.c_str(), key.c_str(), nullptr);
		if (val) {
			try { result = std::stof(val); }
			catch (...) {}
		}
		});
	return result;
}

int IniConfig::ReadInt(const Text::String& section, const Text::String& key,
	int defaultValue) {
	// SimpleIni 提供了直接的 GetLongValue
	long result = defaultValue;
	WithIniReadOnly([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		result = ini.GetLongValue(section.c_str(), key.c_str(), (long)defaultValue);
		});
	return (int)result;
}

bool IniConfig::ReadBool(const Text::String& section, const Text::String& key,
	bool defaultValue) {
	// SimpleIni 提供了直接的 GetBoolValue，支持 true/false/1/0/yes/no
	bool result = defaultValue;
	WithIniReadOnly([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		result = ini.GetBoolValue(section.c_str(), key.c_str(), defaultValue);
		});
	return result;
}

// ─── Write ───────────────────────────────────────────────────────

bool IniConfig::WriteString(const Text::String& section, const Text::String& key,
	const Text::String& value) {
	return WithIni([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		ini.SetValue(section.c_str(), key.c_str(), value.c_str());
		});
}

bool IniConfig::WriteFloat(const Text::String& section, const Text::String& key,
	float value) {
	return WithIni([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		std::ostringstream oss;
		oss << value;
		ini.SetValue(section.c_str(), key.c_str(), oss.str().c_str());
		});
}

bool IniConfig::WriteInt(const Text::String& section, const Text::String& key,
	int value) {
	return WithIni([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		// SimpleIni 提供了直接的 SetLongValue
		ini.SetLongValue(section.c_str(), key.c_str(), (long)value);
		});
}

bool IniConfig::WriteBool(const Text::String& section, const Text::String& key,
	bool value) {
	return WithIni([&](void* pIni) {
		CSimpleIniA& ini = *(CSimpleIniA*)pIni;
		ini.SetBoolValue(section.c_str(), key.c_str(), value);
		});
}



