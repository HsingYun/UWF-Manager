#pragma once

// WmiRow 上的通用"取值+默认值"小工具。
// UwfFilter / UwfVolume / UwfOverlay* / UwfRegistryFilter 都会用到。
// 不想在每个模块里各写一份，这里集中收口。

#include <string>

#include "WmiClient.h"

namespace uwf::rowutil {

bool getBool(const WmiRow& r, const std::string& key, bool def = false);
int32_t getInt(const WmiRow& r, const std::string& key, int32_t def = 0);
uint32_t getUInt(const WmiRow& r, const std::string& key, uint32_t def = 0);
uint64_t getUInt64(const WmiRow& r, const std::string& key, uint64_t def = 0);
std::string getString(const WmiRow& r, const std::string& key);

// 把盘符规整成 "C:" 形式（大写、冒号结尾、只保留前两字符）。
std::string normalizeDriveLetter(const std::string& raw);

// 从 EmbeddedInstance 回传的 __MOF 文本中抠出某个属性的值。
std::string extractFromMof(const std::string& mof, const std::string& propName);

// 某些 GetExclusions 实现把结果放在字段上，另一些只塞进 __MOF。两种都
// fallback。
std::string readExcludedKey(const WmiRow& r, const std::string& propName);

// 把一整行以 "k=v, k=v..." 形式写进调试日志（UWF_LOG_D）。
void dumpRow(const char* tag, const WmiRow& r);

}  // namespace uwf::rowutil
