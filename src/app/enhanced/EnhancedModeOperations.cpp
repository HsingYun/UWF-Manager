// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "EnhancedModeOperations.h"

#include <algorithm>
#include <exception>
#include <utility>

#include "app/common/ApplicationText.h"
#include "app/registry/RegistryTreeCommitter.h"
#include "app/uwf/UwfFilter.h"
#include "shared/Log.h"

namespace uwf::app {

namespace {
QString summarizeFailures(const RegistryCommitResult& result) {
  if (result.succeeded()) return {};
  QString detail = ApplicationText::tr("%1 registry operation(s) failed.").arg(result.failures.size());
  constexpr std::size_t kVisible = 4;
  for (std::size_t index = 0; index < std::min(kVisible, result.failures.size()); ++index) {
    const auto& failure = result.failures[index];
    detail += QStringLiteral("\n%1: %2").arg(QString::fromStdString(failure.target.key), QString::fromStdString(failure.detail));
  }
  return detail;
}

}  // namespace

EnhancedModeOperations::EnhancedModeOperations(std::shared_ptr<EnhancedModeServiceControl> control, wmi::WmiOperations& session, const UwfCapability capability)
    : m_serviceControl(std::move(control)), m_session(session.threadAccessor()), m_capability(capability) {
  if (!m_serviceControl) throw std::invalid_argument("enhanced mode service control is required");
}

EnhancedModeStatus EnhancedModeOperations::status() const { return m_serviceControl->query(); }

EnhancedModeChangeResult EnhancedModeOperations::enable(const QString& serviceDescription) {
  m_serviceControl->installAndStart(serviceDescription);
  const auto warning = persistInstallation();
  return {status(), warning};
}

EnhancedModeChangeResult EnhancedModeOperations::start() {
  m_serviceControl->start();
  return {status(), {}};
}

EnhancedModeChangeResult EnhancedModeOperations::disable() {
  const auto persistencePlan = prepareRemovalPersistence();
  auto removal = m_serviceControl->stopAndMarkForDeletion();
  QString persistenceWarning = persistencePlan ? persistRemoval(*persistencePlan) : QString{};
  removal.barrier.reset();

  if (!persistencePlan && removal.state == EnhancedModeServiceControl::RemovalState::Absent) {
    try {
      // 只有 SCM 明确认定服务不存在时，这棵键才是可直接清理的孤儿。
      // 已标记删除的服务键仍归 SCM 所有，不能与 SCM 的延迟删除竞争。
      m_serviceControl->deleteRegistryRemnants();
    } catch (const std::exception& error) {
      UWF_LOG_E("enhanced") << "enhanced mode registry remnant cleanup failed: error=" << error.what();
      persistenceWarning = QString::fromUtf8(error.what());
    } catch (...) {
      UWF_LOG_E("enhanced") << "enhanced mode registry remnant cleanup failed: error=non-standard-exception";
      persistenceWarning = ApplicationText::tr("The operation failed with an unknown error.");
    }
  }
  return {status(), std::move(persistenceWarning)};
}

QString EnhancedModeOperations::persistInstallation() const {
  if (m_capability != UwfCapability::Available) return {};
  bool registryOwnsFailure = false;
  try {
    provider::UwfFilter filter(m_session());
    if (!filter.read().currentEnabled) return {};

    RegistryTreeCommitter registry(m_session());
    const auto targets = m_serviceControl->planRegistryCommit();
    registryOwnsFailure = true;
    const auto result = registry.commit(targets);
    registryOwnsFailure = false;
    return summarizeFailures(result);
  } catch (const std::exception& error) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode installation persistence failed: error=" << error.what();
    return QString::fromUtf8(error.what());
  } catch (...) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode installation persistence failed: error=non-standard-exception";
    return ApplicationText::tr("The operation failed with an unknown error.");
  }
}

std::optional<std::vector<RegistryCommitTarget>> EnhancedModeOperations::prepareRemovalPersistence() const {
  if (m_capability != UwfCapability::Available) return std::nullopt;
  try {
    provider::UwfFilter filter(m_session());
    if (!filter.read().currentEnabled) return std::nullopt;
    return m_serviceControl->planRegistryDeletion();
  } catch (const std::exception& error) {
    UWF_LOG_E("enhanced") << "enhanced mode removal persistence preparation failed: error=" << error.what();
    throw;
  } catch (...) {
    UWF_LOG_E("enhanced") << "enhanced mode removal persistence preparation failed: error=non-standard-exception";
    throw;
  }
}

QString EnhancedModeOperations::persistRemoval(const std::vector<RegistryCommitTarget>& deletionPlan) const {
  if (deletionPlan.empty()) return {};
  bool registryOwnsFailure = false;
  try {
    RegistryTreeCommitter registry(m_session());
    registryOwnsFailure = true;
    const auto result = registry.commitDeletion(deletionPlan, [control = m_serviceControl](const RegistryCommitTarget& target) {
      return control->registryTargetExists(target);
    });
    registryOwnsFailure = false;
    return summarizeFailures(result);
  } catch (const std::exception& error) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode removal persistence failed: error=" << error.what();
    return QString::fromUtf8(error.what());
  } catch (...) {
    if (!registryOwnsFailure) UWF_LOG_E("enhanced") << "enhanced mode removal persistence failed: error=non-standard-exception";
    return ApplicationText::tr("The operation failed with an unknown error.");
  }
}

}  // namespace uwf::app
