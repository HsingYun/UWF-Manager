#include "RegistryExclusionPolicy.h"

#include <string_view>

#include "../util/RegistryKey.h"
#include "../util/StringUtil.h"
#include "Config.h"

namespace uwf::core {

RegExclusionClass classifyRegistryExclusion(const std::string& key) {
  const std::string upper = toUpperAscii(regkey::normalize(key));
  if (upper.empty()) return RegExclusionClass::OutOfScope;

  // 黑名单：$MACHINE.ACC。它在 SECURITY 下本可通过白名单，必须先单独挡掉。
  if (std::string_view(upper) == config::kForbiddenRegistryKeyMachineAccount) return RegExclusionClass::MachineAccount;

  // 白名单：6 前缀之一的真子键（前缀以 '\' 结尾，故命中后还须多至少一个字符）。
  for (const std::string_view prefix : config::kAllowedRegistryRootPrefixes) {
    if (upper.size() > prefix.size() && upper.starts_with(prefix)) return RegExclusionClass::Allowed;
  }

  // 非真子键，但 key 是某允许前缀的祖先（或正好等于前缀去掉尾部 '\'）→ 子树里
  // 有合法 key，可展开不可选。withSep = key + '\'：
  //   - prefix 以 withSep 开头 ⇒ key 是 prefix 的祖先；
  //   - withSep 以 prefix 开头 ⇒ key 正好等于 prefix 去尾。
  const std::string withSep = upper + '\\';
  for (const std::string_view prefix : config::kAllowedRegistryRootPrefixes) {
    if (prefix.starts_with(withSep) || withSep.starts_with(prefix)) return RegExclusionClass::Container;
  }

  return RegExclusionClass::OutOfScope;
}

}  // namespace uwf::core
