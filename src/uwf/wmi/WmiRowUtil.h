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

#include <optional>
#include <string>
#include <vector>

#include "WmiClient.h"

namespace uwf::rowutil {

// 把 ExecMethod 返回的某个 out 数组（GetExclusions 的 "ExcludedFiles" /
// "ExcludedKeys" 等）逐元素解析成 T。parseItem(const WmiRow&) 返回
// std::optional<T>；nullopt 表示该元素无效，整个响应随即失败。
// WMI provider 对空数组通常返回同名 VT_NULL 标量，而不是零长度 SAFEARRAY。
// 两种形式都映射为空 vector；字段真正缺失或任一元素无法解析才返回 nullopt。
// 这样既保留 provider 的合法空集合语义，也不会把不完整响应冒充为空结果。
template <class T, class ParseItem>
std::optional<std::vector<T>> readArrayOutput(const WmiMethodResult& r, const char* arrayKey, ParseItem parseItem, std::string* error = nullptr) {
  std::vector<T> out;
  const auto it = r.outArrays.find(arrayKey);
  if (it == r.outArrays.end()) {
    const auto nullIt = r.outParams.find(arrayKey);
    if (nullIt != r.outParams.end() && !nullIt->second.isValid()) return out;
    if (error) *error = std::string("required WMI output array is missing: ") + arrayKey;
    return std::nullopt;
  }
  out.reserve(it->second.size());
  for (const auto& item : it->second) {
    auto parsed = parseItem(item);
    if (!parsed) {
      if (error && error->empty()) *error = std::string("invalid element in WMI output array: ") + arrayKey;
      return std::nullopt;
    }
    out.push_back(std::move(*parsed));
  }
  return out;
}

std::optional<bool> requireBool(const WmiRow& r, const std::string& key, std::string* error = nullptr);
std::optional<int32_t> requireInt(const WmiRow& r, const std::string& key, std::string* error = nullptr);
std::optional<uint32_t> requireUInt(const WmiRow& r, const std::string& key, std::string* error = nullptr);
std::optional<uint64_t> requireUInt64(const WmiRow& r, const std::string& key, std::string* error = nullptr);
std::optional<std::string> requireString(const WmiRow& r, const std::string& key, std::string* error = nullptr, bool allowEmpty = true);
// EmbeddedInstance 既可能展开成普通字段，也可能只以 __MOF 文本返回。两种
// provider 表示统一严格读取；缺失、类型错误或空值均返回 nullopt。
std::optional<std::string> requireEmbeddedString(const WmiRow& r, const std::string& propName, std::string* error = nullptr);

// 从 EmbeddedInstance 回传的 __MOF 文本中抠出某个属性的值。
std::string extractFromMof(const std::string& mof, const std::string& propName);

// 把一整行以 "k=v, k=v..." 形式写进调试日志（UWF_LOG_D）。
void dumpRow(const char* tag, const WmiRow& r);

}  // namespace uwf::rowutil
