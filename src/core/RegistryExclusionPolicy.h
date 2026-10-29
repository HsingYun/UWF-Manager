// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 判定归一化键的排除资格，供录入与选择器共用；不包含语法校验。
// 仅允许指定顶层键的子键，并优先禁止 $MACHINE.ACC。

#include <string>

namespace uwf::core {

enum class RegExclusionClass {
  Allowed,         // 是某允许前缀的真子键 → 可排除 / 可选
  MachineAccount,  // 命中 $MACHINE.ACC 黑名单 → 禁
  Container,       // 不是真子键，但是某允许前缀的祖先（或正好等于前缀去尾）→ 可展开、不可选
  OutOfScope,      // 完全不在白名单范围 → 禁
};

// 内部先 regkey::normalize（简写 hive 展开成长写）+ ASCII 大写，再对照白 / 黑名单。
[[nodiscard]] RegExclusionClass classifyRegistryExclusion(const std::string& key);

}  // namespace uwf::core
