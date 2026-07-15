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

// UWF_RegistryFilter —— 按 CurrentSession 存在 2 个实例
// （true 为当前会话，false 为下次会话）。
//
// 字段：PersistDomainSecretKey / PersistTSCAL
// 方法：AddExclusion / RemoveExclusion / FindExclusion / GetExclusions /
//       CommitRegistry / CommitRegistryDeletion

#include <optional>
#include <string>
#include <vector>

#include "../wmi/WmiClient.h"
#include "../wmi/WmiResult.h"
#include "Types.h"

namespace uwf::api {

class UwfRegistryFilter {
 public:
  explicit UwfRegistryFilter(WmiSession& session) : m_session(session) {}

  std::vector<api::RegistryFilterRow> readAll(std::string* error = nullptr) const;

  [[nodiscard]] std::optional<api::RegistryFilterRow> read(bool currentSession, std::string* error = nullptr) const;

  [[nodiscard]] WmiResult addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const;
  [[nodiscard]] WmiResult removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const;

  // 写 UWF_RegistryFilter 的两个全局持久化开关。该类没有对应的 Set* 方法，只能
  // 整实例 PutInstance——本类属性只有 CurrentSession（键）+ 这两个布尔，故必须
  // 同时给出两个值（未改动的那个由调用方用现值兜底）。row 用于定位目标会话。
  [[nodiscard]] WmiResult setPersistFlags(const api::RegistryFilterRow& row, bool persistDomainSecretKey, bool persistTSCAL) const;

  // 返回 true/false；error 非空说明调用失败。
  std::optional<bool> findExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error = nullptr) const;

  std::optional<std::vector<api::ExcludedRegistryKey>> getExclusions(const api::RegistryFilterRow& row, std::string* error = nullptr) const;

  // valueName 为空串 = 提交该键的「默认值」(Default)——CommitRegistry 只能逐个值
  // 提交，没有「提交整个键」的能力（实机验证见 knowledge/reference/11-uwf-api.html
  // 的 CommitRegistry 一节）。结果归类用 commitOutcome()：指定的值（valueName
  // 为空时即默认值）在注册表中不存在时 UWF 返回 WBEM_E_NOT_FOUND，归 Skipped。
  [[nodiscard]] WmiResult commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

  // valueName 为空串 = 删除整项。
  [[nodiscard]] WmiResult commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf::api
