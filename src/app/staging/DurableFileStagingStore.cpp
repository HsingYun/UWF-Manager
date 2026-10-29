// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DurableFileStagingStore.h"

#include <stdexcept>
#include <utility>

#include "app/api/ConfigurationPlan.h"
#include "app/paths/PathPolicy.h"
#include "app/uwf/UwfFilter.h"
#include "app/uwf/UwfRegistryFilter.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
namespace uwf::app {
namespace {

[[nodiscard]] std::vector<std::string> exclusionPaths(const app::provider::UwfRegistryFilter& registry, const app::provider::RegistryFilterRow& row) {
  std::vector<std::string> exclusions;
  for (const auto& exclusion : registry.getExclusions(row)) exclusions.push_back(exclusion.registryKey);
  return exclusions;
}

[[nodiscard]] bool isCovered(const std::vector<std::string>& exclusions) {
  return !app::paths::findCoveringExclusion(exclusions, std::string(app::kFileStagingRegistryRoot)).empty();
}

}  // namespace

DurableFileStagingStore::DurableFileStagingStore(std::shared_ptr<FileStagingStorage> storage, wmi::WmiOperations& session, UwfCapability capability)
    : m_storage(std::move(storage)), m_session(session.threadAccessor()), m_capability(capability) {}
QList<FileStagingEntry> DurableFileStagingStore::load() const { return m_storage->load(); }
void DurableFileStagingStore::replace(const QList<FileStagingEntry>& entries) {
  if (entries.isEmpty()) {
    clearCurrent();
    return;
  }
  m_storage->replace(entries);
  preserveCurrent();
}
void DurableFileStagingStore::reconcileDurability() {
  const auto entries = load();
  if (entries.isEmpty() || m_capability != UwfCapability::Available) return;
  preserveCurrent();
  provider::UwfRegistryFilter registry(m_session());
  const auto next = registry.read(provider::Session::Next);
  core::UwfSnapshot snapshot;
  snapshot.next.registryExclusions = exclusionPaths(registry, next);
  const auto plan = planConfiguration({}, std::move(snapshot), FileStagingChanges{{}, entries, true});
  for (const auto& key : plan.changes.addRegistryExclusions) registry.addExclusion(next, key);
}
void DurableFileStagingStore::preserveCurrent() {
  if (m_capability != app::UwfCapability::Available) return;

  app::provider::UwfFilter filter(m_session());
  const auto filterState = filter.read();
  app::provider::UwfRegistryFilter registry(m_session());
  const auto rows = registry.readAll();
  const auto* current = app::provider::findBySession(rows, app::provider::Session::Current);
  if (!current) throw std::runtime_error("file staging durability requires a current UWF registry filter record");

  const auto currentExclusions = filterState.currentEnabled ? exclusionPaths(registry, *current) : std::vector<std::string>{};
  // 当前会话已受保护且本键尚未由排除项覆盖时，刚写入的列表仍位于覆盖层；
  // 必须先提交现有值，再把根键加入下一会话排除，否则重启后会得到一条排除项
  // 却丢失它本应保护的暂存列表。
  if (filterState.currentEnabled && !isCovered(currentExclusions)) {
    try {
      registry.commitRegistry(*current, std::string(app::kFileStagingRegistryKey), std::string(app::kFileStagingEntriesValue));
    } catch (const app::wmi::WmiException& error) {
      // 启动对账时该值可能已经位于真实注册表、当前覆盖层没有对应改动。
      // 此时 provider 返回 NotFound，但刚才的 load() 已确认列表当前可读；
      // 将其视为幂等完成，继续补齐下一会话排除关系。
      if (!error.hasCode(app::wmi::WmiErrorCode::NotFound)) throw;
    }
  }
}

void DurableFileStagingStore::clearCurrent() {
  if (m_capability != app::UwfCapability::Available) {
    m_storage->replace({});
    return;
  }

  app::provider::UwfFilter filter(m_session());
  const auto filterState = filter.read();
  app::provider::UwfRegistryFilter registry(m_session());
  const auto rows = registry.readAll();
  const auto* current = app::provider::findBySession(rows, app::provider::Session::Current);
  if (!current) throw std::runtime_error("file staging cleanup requires a current UWF registry filter record");

  const auto currentExclusions = filterState.currentEnabled ? exclusionPaths(registry, *current) : std::vector<std::string>{};
  if (m_storage->exists()) {
    if (!filterState.currentEnabled || isCovered(currentExclusions)) {
      // 筛选器未运行时直接删除即落盘；当前键已排除时写入同样绕过覆盖层，
      // CommitRegistryDeletion 反而没有可提交的覆盖层目标。
      m_storage->replace({});
    } else {
      // 不依赖 CommitRegistryDeletion 对非空父键的递归删除；受保护系统
      // 实测该调用可能失败，因此保留后序逐键提交删除。只在
      // 整棵树都处理成功后移除下一会话排除项，否则重启会恢复出一份没有
      // 持久化保障的半清理状态。
      for (const auto& target : m_storage->deletionPlan()) {
        try {
          registry.commitRegistryDeletion(*current, target.key, target.valueName);
        } catch (const app::wmi::WmiException& error) {
          // A target removed since planning already satisfies this deletion.
          if (!error.hasCode(app::wmi::WmiErrorCode::NotFound)) throw;
        }
      }
    }
  }
}
}  // namespace uwf::app
