/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// WmiRow 上的通用"取值+默认值"小工具。
// UwfFilter / UwfVolume / UwfOverlay* / UwfRegistryFilter 都会用到。
// 不想在每个模块里各写一份，这里集中收口。

#include <optional>
#include <string>
#include <vector>

#include "WmiClient.h"

namespace uwf::rowutil {

// 把 ExecMethod 返回的某个 out 数组（GetExclusions 的 "ExcludedFiles" /
// "ExcludedKeys" 等）逐元素解析成 T。parseItem(const WmiRow&) 返回
// std::optional<T>，nullopt 表示跳过该元素（如空 FileName / RegistryKey）。
// 找不到该数组名 → 返回空 vector。把"find 数组 + reserve + 跳空 loop"这套
// 骨架收口一处，各 getExclusions 只需给出数组名与单元素解析逻辑。
template <class T, class ParseItem>
std::vector<T> readOutArray(const WmiMethodResult& r, const char* arrayKey, ParseItem parseItem) {
  std::vector<T> out;
  const auto it = r.outArrays.find(arrayKey);
  if (it == r.outArrays.end()) return out;
  out.reserve(it->second.size());
  for (const auto& item : it->second) {
    if (auto parsed = parseItem(item)) out.push_back(std::move(*parsed));
  }
  return out;
}

bool getBool(const WmiRow& r, const std::string& key, bool def = false);
int32_t getInt(const WmiRow& r, const std::string& key, int32_t def = 0);
uint32_t getUInt(const WmiRow& r, const std::string& key, uint32_t def = 0);
uint64_t getUInt64(const WmiRow& r, const std::string& key, uint64_t def = 0);
std::string getString(const WmiRow& r, const std::string& key);

// 从 EmbeddedInstance 回传的 __MOF 文本中抠出某个属性的值。
std::string extractFromMof(const std::string& mof, const std::string& propName);

// 某些 GetExclusions 实现把结果放在字段上，另一些只塞进 __MOF。两种都
// fallback。
std::string readExcludedKey(const WmiRow& r, const std::string& propName);

// 把一整行以 "k=v, k=v..." 形式写进调试日志（UWF_LOG_D）。
void dumpRow(const char* tag, const WmiRow& r);

}  // namespace uwf::rowutil
