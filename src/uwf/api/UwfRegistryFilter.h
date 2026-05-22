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
#include "CommitResult.h"
#include "Types.h"

namespace uwf {

class UwfRegistryFilter {
 public:
  explicit UwfRegistryFilter(WmiSession& session) : m_session(session) {}

  std::vector<api::RegistryFilterRow> readAll(std::string* error = nullptr) const;

  [[nodiscard]] std::optional<api::RegistryFilterRow> read(bool currentSession, std::string* error = nullptr) const;

  bool addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error = nullptr) const;
  bool removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error = nullptr) const;

  // 返回 true/false；error 非空说明调用失败。
  std::optional<bool> findExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error = nullptr) const;

  std::vector<api::ExcludedRegistryKey> getExclusions(const api::RegistryFilterRow& row, std::string* error = nullptr) const;

  // valueName 为空串 = 提交整项。结果分 Ok / Skipped / Failed（见 CommitResult）：
  // 注册表项在 overlay 里没有待提交改动时 UWF 返回 WBEM_E_NOT_FOUND，归 Skipped——
  // 键值已与磁盘一致，不算错误。
  CommitResult commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

  // valueName 为空串 = 删除整项。
  CommitResult commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const;

 private:
  WmiSession& m_session;
};

}  // namespace uwf
