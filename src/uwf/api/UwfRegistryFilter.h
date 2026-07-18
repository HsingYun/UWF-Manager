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

#include <string>
#include <vector>

#include "../wmi/WmiClient.h"
#include "Types.h"

namespace uwf::api {

class UwfRegistryFilter {
 public:
  explicit UwfRegistryFilter(WmiOperations& session) : m_session(session) {}

  std::vector<api::RegistryFilterRow> readAll() const;

  [[nodiscard]] api::RegistryFilterRow read(api::Session session) const;

  void addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const;
  void removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const;

  // 写 UWF_RegistryFilter 的两个全局持久化开关。该类没有对应的 Set* 方法，只能
  // 整实例 PutInstance——本类属性只有 CurrentSession（键）+ 这两个布尔，故必须
  // 同时给出两个值（未改动的那个由调用方用现值兜底）。row 用于定位目标会话。
  void setPersistence(const api::RegistryFilterRow& row, api::RegistryPersistence persistence) const;

  bool findExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const;

  std::vector<api::ExcludedRegistryKey> getExclusions(const api::RegistryFilterRow& row) const;

  // valueName 为空串 = 提交该键的「默认值」(Default)——CommitRegistry 只能逐个值
  // 提交，没有「提交整个键」的能力（实机验证见 knowledge/reference/11-uwf-api.html
  // 的 CommitRegistry 一节）。指定的值（valueName 为空时即默认值）不存在时，
  // provider 抛出的 WmiProviderError 保留 WBEM_E_NOT_FOUND，由批处理边界归类为 Skipped。
  void commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

  // valueName 为空串 = 删除整项。
  void commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

 private:
  WmiOperations& m_session;
};

}  // namespace uwf::api
