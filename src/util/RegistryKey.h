#pragma once

// 注册表键工具：把注册表键归一成"长写 hive + 规范大小写"的标准形态。
// 用户可能键入简写（HKLM\...）或长写（HKEY_LOCAL_MACHINE\...）——统一归一后，
// 对外展示口径一致，去重也不会把同一个键的两种写法当成两条。
//
// 归一到长写而非简写：UWF 官方文档（UWF_RegistryFilter.AddExclusion /
// UWF_ExcludedRegistryKey）用的就是 HKEY_LOCAL_MACHINE\... 这种长写形式，
// AddExclusion 也接受长写，故归一到长写对"展示"和"提交给 WMI"都安全。
//
// 不依赖 Qt，core / uwf / ui 各层均可使用（UI 层在边界处做 QString 转换）。

#include <string>

namespace uwf::regkey {

// 把注册表键归一：
//   - 开头的 hive 简写（HKLM / HKCU / HKCR / HKU / HKCC，大小写不敏感）展开成
//     长写（HKEY_LOCAL_MACHINE 等）；本就是长写的则规范其大小写。
//   - 顺带 trim、去掉结尾多余的反斜杠。
//   - hive 之后的子键路径原样保留——注册表子键大小写不敏感，但展示保留用户书写。
//   - hive 无法识别时整串原样返回（仅做 trim / 去尾反斜杠），交由调用方的
//     合法性校验去拒绝。
std::string normalize(const std::string& key);

}  // namespace uwf::regkey
