// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 注册表访问与键名归一化；接受简写及完整 hive 名，统一使用 64 位视图。
// 头文件不依赖 Qt 或平台头。

#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "app/api/RegistryValueInfo.h"
#include "shared/RegistryPath.h"

namespace uwf::regkey {

// 判断一个注册表键本身是否存在。key 内部会先 normalize，简写或长写均可。
// 只有 Win32 明确返回 not-found 才是 false；无效 hive、权限不足或其他探测
// 失败抛 std::system_error，不得伪装成“不存在”。
[[nodiscard]] bool keyExists(std::string_view key);

// 判断该注册表键下名为 valueName 的值是否存在；valueName 为空表示键的默认值
// (Default)。key 内部会先 normalize，简写或长写均可；键或值明确不存在
// 返回 false，其他探测失败抛 std::system_error。
[[nodiscard]] bool valueExists(std::string_view key, std::string_view valueName);

// 读取注册表字符串值（REG_SZ / REG_EXPAND_SZ）。键 / 值不存在返回 nullopt；
// 已存在的空字符串返回 engaged optional；类型不符或其他读取失败抛异常。
[[nodiscard]] std::optional<std::string> readString(std::string_view key, std::string_view valueName);

// 读取注册表 DWORD 值（REG_DWORD）。键 / 值不存在返回 nullopt；合法的 0 返回
// engaged optional；类型或长度不符以及其他读取失败抛异常。
[[nodiscard]] std::optional<std::uint32_t> readDword(std::string_view key, std::string_view valueName);

// 读取 REG_MULTI_SZ。键 / 值不存在返回 nullopt；合法的空列表返回 engaged
// optional。数据类型错误、缺少双 NUL 终止符或其他读取失败抛异常。
[[nodiscard]] std::optional<std::vector<std::string>> readMultiString(std::string_view key, std::string_view valueName);

// 创建（或打开）key，并以单个 REG_MULTI_SZ 值原子替换整个字符串列表。
// 空列表写成合法的双 NUL 数据；字符串内部含 NUL 时拒绝写入。
void writeMultiString(std::string_view key, std::string_view valueName, const std::vector<std::string>& values);

// 递归删除 key 及其全部子键和值。键明确不存在时视为幂等成功；权限不足、
// hive 非法或其它 Win32 错误抛 std::system_error。始终操作 64 位注册表视图。
void deleteTree(std::string_view key);

// 列出 key 的直接子键名（仅名字，不含完整路径）。键不存在返回空；无效 hive、
// 权限不足或枚举失败抛 std::system_error。
[[nodiscard]] std::vector<std::string> subkeyNames(std::string_view key, std::stop_token stop = {});
void visitSubkeyNames(std::string_view key, const std::function<void(const std::string&)>& discover, std::stop_token stop = {});

// 通过 RegQueryInfoKeyW 查询子键数，避免完整枚举。键不存在返回 false，其他失败抛 std::system_error。
[[nodiscard]] bool hasSubkeys(std::string_view key);

// 列出 key 上的全部值名；默认值 (Default) 存在时其名为空串 "" 也会列出。
// 键不存在返回空；其他枚举失败抛 std::system_error。
[[nodiscard]] std::vector<std::string> valueNames(std::string_view key, std::stop_token stop = {});
void visitValueNames(std::string_view key, const std::function<void(const std::string&)>& discover, std::stop_token stop = {});

// 列出值名、Win32 类型码和有界预览数据；仅需名称时使用 valueNames。
[[nodiscard]] std::vector<app::RegistryValueInfo> values(std::string_view key);

[[nodiscard]] std::vector<std::string> rootHiveLongNames();

// 收集 key 子树里的所有键（含 key 自身），后序排列——最深的子键在前、key 在最末，
// 正好是「递归删除」要的顺序（逐个删时每个键被处理时都已是叶子）。返回归一化后的
// 长写完整键路径。
[[nodiscard]] std::vector<std::string> collectKeyTree(const std::string& key);

}  // namespace uwf::regkey
