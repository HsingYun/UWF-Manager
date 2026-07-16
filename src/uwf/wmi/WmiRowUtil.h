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

// WmiRow 的严格字段解码工具。业务字段缺失或类型不符必须显式失败，禁止
// 通过默认 0 / false / 空字符串把损坏响应伪装成有效状态。

#include <string>
#include <vector>

#include "WmiClient.h"
#include "WmiException.h"

namespace uwf::rowutil {

// 把 ExecMethod 返回的某个 out 数组（GetExclusions 的 "ExcludedFiles" /
// "ExcludedKeys" 等）逐元素解析成 T。parseItem(const WmiRow&) 直接返回 T；
// 任一元素无效时抛出解码异常，整个响应不会部分提交。
// WMI provider 对空数组通常返回同名 VT_NULL 标量，而不是零长度 SAFEARRAY。
// 两种形式都映射为空 vector；字段真正缺失或任一元素无法解析则抛出异常。
// 这样既保留 provider 的合法空集合语义，也不会把不完整响应冒充为空结果。
template <class T, class ParseItem>
std::vector<T> readArrayOutput(const WmiMethodOutput& output, const char* arrayKey, ParseItem parseItem) {
  std::vector<T> out;
  const auto it = output.arrays.find(arrayKey);
  if (it == output.arrays.end()) {
    const auto nullIt = output.values.find(arrayKey);
    if (nullIt != output.values.end() && !nullIt->second.isValid()) return out;
    throw WmiProtocolError("decode method output", std::string("required output array is missing: ") + arrayKey);
  }
  out.reserve(it->second.size());
  for (const auto& item : it->second) {
    out.push_back(parseItem(item));
  }
  return out;
}

enum class EmptyString { Allow, Reject };

bool requireBool(const WmiRow& r, const std::string& key);
int32_t requireInt(const WmiRow& r, const std::string& key);
uint32_t requireUInt(const WmiRow& r, const std::string& key);
uint64_t requireUInt64(const WmiRow& r, const std::string& key);
std::string requireString(const WmiRow& r, const std::string& key, EmptyString empty = EmptyString::Allow);
// EmbeddedInstance 既可能展开成普通字段，也可能只以 __MOF 文本返回。两种
// provider 表示统一严格读取；缺失、类型错误或空值均抛出异常。
std::string requireEmbeddedString(const WmiRow& r, const std::string& propName);

// 从 EmbeddedInstance 回传的 __MOF 文本中抠出某个属性的值。
std::string extractFromMof(const std::string& mof, const std::string& propName);

// 把一整行以 "k=v, k=v..." 形式写进调试日志（UWF_LOG_D）。
// 诊断输出不得改变业务读取结果；格式化或日志缓冲失败时仅丢弃本条日志。
void dumpRow(const char* tag, const WmiRow& r) noexcept;

}  // namespace uwf::rowutil
