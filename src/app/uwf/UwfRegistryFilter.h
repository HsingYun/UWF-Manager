// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// UWF_RegistryFilter —— 按 CurrentSession 存在 2 个实例
// （true 为当前会话，false 为下次会话）。
//
// 字段：PersistDomainSecretKey / PersistTSCAL
// 方法：AddExclusion / RemoveExclusion / FindExclusion / GetExclusions /
//       CommitRegistry / CommitRegistryDeletion

#include <string>
#include <vector>

#include "UwfProviderTypes.h"
#include "app/ports/WmiOperations.h"

namespace uwf::app::provider {

class UwfRegistryFilter {
 public:
  explicit UwfRegistryFilter(wmi::WmiOperations& session) : m_session(session) {}

  std::vector<provider::RegistryFilterRow> readAll() const;

  [[nodiscard]] provider::RegistryFilterRow read(provider::Session session) const;

  void addExclusion(const provider::RegistryFilterRow& row, const std::string& registryKey) const;
  void removeExclusion(const provider::RegistryFilterRow& row, const std::string& registryKey) const;

  // 持久化开关通过整实例更新，必须同时提供两个值；未修改项由调用方保留。
  void setPersistence(const provider::RegistryFilterRow& row, provider::RegistryPersistence persistence) const;

  bool findExclusion(const provider::RegistryFilterRow& row, const std::string& registryKey) const;

  std::vector<provider::ExcludedRegistryKey> getExclusions(const provider::RegistryFilterRow& row) const;

  // 空值名表示默认值；CommitRegistry 仅逐值提交。值不存在时保留 provider 的 NotFound 错误，由批处理层分类。
  void commitRegistry(const provider::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

  // valueName 为空串 = 删除整项。
  void commitRegistryDeletion(const provider::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

 private:
  wmi::WmiOperations& m_session;
};

}  // namespace uwf::app::provider
