#pragma once

// 注册表键工具：键归一、存在性判断、值读取。
//
// 用户可能键入简写（HKLM\...）或长写（HKEY_LOCAL_MACHINE\...）——统一归一后，
// 对外展示口径一致，去重也不会把同一个键的两种写法当成两条。
//
// 全项目的注册表读取都收口在这里：valueExists / readString / readDword 让各处
// 不再各写一份 RegOpenKeyExW + RegQueryValueExW。键参数接受简写或长写 hive。
//
// 头文件不依赖 Qt、不引入平台头（实现在 .cpp 里用 Windows 注册表 API）。

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace uwf::regkey {

// 把注册表键归一：
//   - 开头的 hive 简写（HKLM / HKCU / HKCR / HKU / HKCC，大小写不敏感）展开成
//     长写（HKEY_LOCAL_MACHINE 等）；本就是长写的则规范其大小写。
//   - 顺带 trim、去掉结尾多余的反斜杠。
//   - hive 之后的子键路径原样保留——注册表子键大小写不敏感，但展示保留用户书写。
//   - hive 无法识别时整串原样返回（仅做 trim / 去尾反斜杠），交由调用方的
//     合法性校验去拒绝。
std::string normalize(const std::string& key);

// 判断一个注册表键本身是否存在。key 内部会先 normalize，简写或长写均可。
[[nodiscard]] bool keyExists(std::string_view key);

// 判断该注册表键下名为 valueName 的值是否存在；valueName 为空表示键的默认值
// (Default)。key 内部会先 normalize，简写或长写均可；无法识别 hive、键不存在、
// 或该值不存在均返回 false。
[[nodiscard]] bool valueExists(std::string_view key, std::string_view valueName);

// 读取注册表字符串值（REG_SZ / REG_EXPAND_SZ）；键 / 值不存在或类型不符返回空串。
[[nodiscard]] std::string readString(std::string_view key, std::string_view valueName);

// 读取注册表 DWORD 值（REG_DWORD）；键 / 值不存在返回 0。
[[nodiscard]] std::uint32_t readDword(std::string_view key, std::string_view valueName);

// 列出 key 的直接子键名（仅名字，不含完整路径）。无法识别 hive / 键不存在 → 空。
[[nodiscard]] std::vector<std::string> subkeyNames(std::string_view key);

// 列出 key 上的全部值名；默认值 (Default) 存在时其名为空串 "" 也会列出。
// 无法识别 hive / 键不存在 → 空。
[[nodiscard]] std::vector<std::string> valueNames(std::string_view key);

// 收集 key 子树里的所有键（含 key 自身），后序排列——最深的子键在前、key 在最末，
// 正好是「递归删除」要的顺序（逐个删时每个键被处理时都已是叶子）。返回归一化后的
// 长写完整键路径。
[[nodiscard]] std::vector<std::string> collectKeyTree(const std::string& key);

}  // namespace uwf::regkey
