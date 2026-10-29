// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// WmiRow 的严格字段解码工具。业务字段缺失或类型不符必须显式失败，禁止
// 通过默认 0 / false / 空字符串把损坏响应伪装成有效状态。

#include <string>
#include <vector>

#include "app/ports/WmiOperations.h"
#include "app/wmi/WmiException.h"

namespace uwf::app::wmi::row {

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
bool requireBoolOrFalseIfNull(const WmiRow& r, const std::string& key);
int32_t requireInt(const WmiRow& r, const std::string& key);
uint32_t requireUInt(const WmiRow& r, const std::string& key);
uint64_t requireUInt64(const WmiRow& r, const std::string& key);
std::string requireString(const WmiRow& r, const std::string& key, EmptyString empty = EmptyString::Allow);
// EmbeddedInstance 既可能展开成普通字段，也可能只以 __MOF 文本返回。两种
// provider 表示统一严格读取；缺失、类型错误或空值均抛出异常。
std::string requireEmbeddedString(const WmiRow& r, const std::string& propName);

// 从 EmbeddedInstance 的 __MOF 文本提取指定属性。
std::string extractFromMof(const std::string& mof, const std::string& propName);

// 把一整行以 "k=v, k=v..." 形式写进调试日志（UWF_LOG_D）。
// 诊断输出不得改变业务读取结果；格式化或日志缓冲失败时仅丢弃本条日志。
void dumpRow(const char* tag, const WmiRow& r) noexcept;

}  // namespace uwf::app::wmi::row
