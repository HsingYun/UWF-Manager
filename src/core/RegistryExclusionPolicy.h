#pragma once

// 注册表排除项的白 / 黑名单判定。UWF 只允许排除 6 个顶层键的「真子键」
// （见 config::kAllowedRegistryRootPrefixes），且明确禁排域机器账户密钥
// $MACHINE.ACC（见 config::kForbiddenRegistryKeyMachineAccount）。
//
// 这套规则原先在两处各写一份：ExclusionListWidget（导入 uwfmgr 脚本里的自由文本
// 键校验 + picker 选完后的防御性复检，要给中文拒绝原因）与 RegistryPickerDialog
// （注册表树节点着色，要三态可选性）。抽到这里共用，避免两份白名单将来漂移。本
// 判定**不含语法校验**（空串 / 非法字符 / 引导反斜杠等留给调用方各自处理），只
// 回答"归一后的键落在白 / 黑名单的哪一类"。
//
// 不依赖 Qt——返回枚举，面向用户的文案 / 控件状态由调用方按 enum 自己决定。

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
