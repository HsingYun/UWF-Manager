// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ConfigurationExecutor.h"

#include <algorithm>
#include <exception>
#include <optional>
#include <set>
#include <utility>

#include "app/api/ConfigurationPlan.h"
#include "app/common/ApplicationText.h"
#include "app/uwf/UwfFilter.h"
#include "app/uwf/UwfOverlay.h"
#include "app/uwf/UwfOverlayConfig.h"
#include "app/uwf/UwfRegistryFilter.h"
#include "app/uwf/UwfVolume.h"
#include "app/wmi/WmiException.h"
#include "core/OverlayWritePlan.h"
#include "core/UwfConstraints.h"
#include "shared/Log.h"

namespace uwf::app {

namespace {

const provider::VolumeRow* findNextVolume(const std::vector<provider::VolumeRow>& rows, const std::string& driveLetter) {
  return provider::findBySession(rows, provider::Session::Next, [&](const provider::VolumeRow& v) { return v.driveLetter == driveLetter; });
}

provider::OverlayType coreTypeToProvider(core::OverlayType t) {
  return t == core::OverlayType::Disk ? provider::OverlayType::Disk : provider::OverlayType::RAM;
}

struct ApplyMessages {
  QString success;
  QString failure;
};

class ApplyJournal final {
 public:
  enum class StepResult { Confirmed, Failed };
  void begin(ConfigurationOperationKind kind, std::string target = {}) {
    m_operations = {{kind, std::move(target)}};
    m_stepEffect = WriteEffect::None;
  }
  void include(ConfigurationOperationKind kind) { m_operations.push_back({kind, {}}); }
  void endOperation() {
    m_operations.clear();
    m_stepEffect = WriteEffect::None;
  }

  template <typename Action>
  StepResult execute(const QString& failureContext, Action&& action) {
    try {
      m_stepEffect = WriteEffect::None;
      action();
      m_stepEffect = WriteEffect::Confirmed;
      m_anyWriteConfirmed = true;
      m_reconciliationRequired = true;
      return StepResult::Confirmed;
    } catch (const std::exception& error) {
      recordError(failureContext, error);
    } catch (...) {
      const QString message = failureContext + ApplicationText::tr(": unknown error");
      UWF_LOG_E("apply") << "operation failed: error=" << message.toStdString();
      recordFailure(message);
    }
    return StepResult::Failed;
  }

  template <typename Action>
  StepResult apply(const ApplyMessages& messages, Action&& action, const bool prerequisiteConfirmed = true) {
    if (!prerequisiteConfirmed) {
      UWF_LOG_E("apply") << "dependent write skipped: context=" << messages.failure.toStdString();
      recordFailure(messages.failure + ": " + ApplicationText::tr("Skipped because a prerequisite operation failed or could not be confirmed."),
                    ConfigurationStepOutcome::Skipped);
      return StepResult::Failed;
    }
    const auto result = execute(messages.failure, std::forward<Action>(action));
    if (result == StepResult::Confirmed) recordSuccess(messages.success);
    return result;
  }

  void recordSuccess(const QString& message) {
    recordSteps(message, ConfigurationStepOutcome::Succeeded);
    ++m_succeededOperations;
    m_lines.push_back(message.toStdString());
  }

  void recordFailure(const QString& message, ConfigurationStepOutcome outcome = ConfigurationStepOutcome::Failed) {
    recordSteps(message, outcome);
    ++m_failedOperations;
    m_lines.push_back(message.toStdString());
  }

  void recordError(const QString& context, const std::exception& error) {
    if (const auto* operation = dynamic_cast<const OperationError*>(&error)) {
      m_stepEffect = operation->writeEffect();
      m_reconciliationRequired |= m_stepEffect != WriteEffect::None;
      m_anyWriteConfirmed |= m_stepEffect == WriteEffect::Confirmed;
    }
    UWF_LOG_E("apply") << "operation failed: context=" << context.toStdString() << " error=" << error.what();
    recordFailure(context + ": " + QString::fromUtf8(error.what()));
  }

  void registrationObserved(const provider::VolumeRegistrationDisposition disposition, const std::string& drive) {
    // ensureNextSessionEntry 只在本轮基线没有 next-session 行时调用。无论条目
    // 来自本次写入还是并发创建，都必须重读；只有本进程创建或确认了不确定写，
    // 才算本批次确认成功并显示重启入口。
    m_reconciliationRequired = true;
    const bool written =
        disposition == provider::VolumeRegistrationDisposition::Created || disposition == provider::VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite;
    m_anyWriteConfirmed |= written;
    m_steps.push_back({drive, ConfigurationStepOutcome::Succeeded, written ? WriteEffect::Confirmed : WriteEffect::None,
                       ConfigurationOperation{ConfigurationOperationKind::VolumeRegistration, drive}});
    endOperation();
  }

  void confirmed() { m_stepEffect = WriteEffect::Confirmed; }
  void requireReconciliation() { m_reconciliationRequired = true; }

  [[nodiscard]] std::vector<ConfigurationStepResult> steps(const ConfigurationPlan& plan) const {
    auto result = m_steps;
    for (const auto& operation : plan.operations()) {
      if (std::ranges::none_of(result, [&](const auto& step) { return step.operation == operation; }))
        result.push_back({ApplicationText::tr("Skipped because a prerequisite operation failed or could not be confirmed.").toStdString(),
                          ConfigurationStepOutcome::Skipped, WriteEffect::None, operation});
    }
    return result;
  }
  [[nodiscard]] bool anyWriteConfirmed() const noexcept { return m_anyWriteConfirmed; }
  [[nodiscard]] bool satisfiesStagingCompletion(const ConfigurationPlan& plan) const {
    return plan.stagingCompletionAlternatives.empty() || std::ranges::any_of(plan.stagingCompletionAlternatives, [&](const auto& operation) {
             return std::ranges::any_of(m_steps,
                                        [&](const auto& step) { return step.operation == operation && step.outcome == ConfigurationStepOutcome::Succeeded; });
           });
  }
  [[nodiscard]] bool reconciliationRequired() const noexcept { return m_reconciliationRequired; }
  [[nodiscard]] const std::vector<std::string>& lines() const noexcept { return m_lines; }
  [[nodiscard]] int succeededOperations() const noexcept { return m_succeededOperations; }
  [[nodiscard]] int failedOperations() const noexcept { return m_failedOperations; }

 private:
  void recordSteps(const QString& message, ConfigurationStepOutcome outcome) {
    if (m_operations.empty())
      m_steps.push_back({message.toStdString(), outcome, m_stepEffect, {}});
    else
      for (const auto& operation : m_operations) m_steps.push_back({message.toStdString(), outcome, m_stepEffect, operation});
    endOperation();
  }
  std::vector<std::string> m_lines;
  std::vector<ConfigurationStepResult> m_steps;
  WriteEffect m_stepEffect = WriteEffect::None;
  std::vector<ConfigurationOperation> m_operations;
  bool m_anyWriteConfirmed = false;
  bool m_reconciliationRequired = false;
  int m_succeededOperations = 0;
  int m_failedOperations = 0;
};

}  // namespace

ConfigurationExecutor::ConfigurationExecutor(wmi::WmiOperations& session, drive::MountLookup mounts, std::shared_ptr<FileStagingDraft> staging,
                                             std::optional<FileStagingChanges> stagingChanges)
    : m_session(session.threadAccessor()), m_mounts(std::move(mounts)), m_staging(std::move(staging)), m_stagingChanges(std::move(stagingChanges)) {}

namespace {
ConfigurationApplyResult executePlan(wmi::WmiOperations& session, const ConfigurationPlan& plan, const FileStagingWriteOperation& writeStaging,
                                     const drive::MountLookup& mounts) {
  const auto& pending = plan.changes;
  const auto& snapshot = plan.snapshot;
  const auto& stagingChanges = plan.staging;
  provider::UwfFilter filter(session);
  provider::UwfOverlay overlayProvider(session);
  provider::UwfOverlayConfig overlayConfig(session);
  provider::UwfVolume volume(session, mounts);
  provider::UwfRegistryFilter registry(session);
  std::optional<FileStagingWriteResult> stagingResult;
  ApplyJournal journal;

  try {
    session.ensureConnected();
  } catch (const std::exception& error) {
    journal.recordError(ApplicationText::tr("✘ Failed to connect to the system"), error);
    return {journal.lines(),
            journal.anyWriteConfirmed(),
            journal.reconciliationRequired(),
            false,
            journal.succeededOperations(),
            journal.failedOperations(),
            journal.steps(plan),
            stagingResult};
  }

  if (stagingChanges) {
    journal.begin(ConfigurationOperationKind::Staging);
    const auto result = journal.apply({.success = ApplicationText::tr("File staging"), .failure = ApplicationText::tr("File staging could not be saved")}, [&] {
      if (!writeStaging) throw std::logic_error("file staging apply requires a prepared write");
      stagingResult = writeStaging();
      if (stagingResult->failure) std::rethrow_exception(stagingResult->failure);
    });
    if (result != ApplyJournal::StepResult::Confirmed)
      return {journal.lines(),
              journal.anyWriteConfirmed(),
              journal.reconciliationRequired(),
              true,
              journal.succeededOperations(),
              journal.failedOperations(),
              journal.steps(plan),
              stagingResult};
  }

  if (pending.setFilterEnabled) {
    try {
      const auto row = filter.read();
      const bool enable = *pending.setFilterEnabled;
      journal.begin(ConfigurationOperationKind::Filter);
      journal.apply({.success = ApplicationText::tr("✓ Filter: %1").arg(enable ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")),
                     .failure = ApplicationText::tr("✘ Failed to %1 filter").arg(enable ? ApplicationText::tr("enable") : ApplicationText::tr("disable"))},
                    [&] {
                      if (enable)
                        filter.enable(row);
                      else
                        filter.disable(row);
                    });
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read filter state"), error);
    }
  }

  // 阈值不区分会话，也不要求筛选器关闭。
  if (pending.setOverlay.warningThresholdMb || pending.setOverlay.criticalThresholdMb) {
    try {
      const auto overlay = overlayProvider.read();
      const core::OverlayConfig baseline{.warningThresholdMb = overlay.warningOverlayThreshold, .criticalThresholdMb = overlay.criticalOverlayThreshold};
      const core::OverlayConfigDelta changes{.type = std::nullopt,
                                             .maximumSizeMb = std::nullopt,
                                             .warningThresholdMb = pending.setOverlay.warningThresholdMb,
                                             .criticalThresholdMb = pending.setOverlay.criticalThresholdMb};
      auto previous = ApplyJournal::StepResult::Confirmed;
      for (const auto& step : core::planOverlayWrites(baseline, changes)) {
        const bool ready = !step.requiresPrevious || previous == ApplyJournal::StepResult::Confirmed;
        if (step.field == core::OverlayField::WarningThreshold) {
          const auto value = *changes.warningThresholdMb;
          journal.begin(ConfigurationOperationKind::WarningThreshold);
          previous = journal.apply(
              {.success = ApplicationText::tr("✓ Overlay warning threshold set to %1 MB").arg(value),
               .failure = ApplicationText::tr("✘ Failed to set warning threshold")},
              [&] { overlayProvider.setWarningThreshold(overlay, value); }, ready);
        } else {
          const auto value = *changes.criticalThresholdMb;
          journal.begin(ConfigurationOperationKind::CriticalThreshold);
          previous = journal.apply(
              {.success = ApplicationText::tr("✓ Overlay critical threshold set to %1 MB").arg(value),
               .failure = ApplicationText::tr("✘ Failed to set critical threshold")},
              [&] { overlayProvider.setCriticalThreshold(overlay, value); }, ready);
        }
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read overlay state"), error);
    }
  }

  // 类型和大小写入下一会话；当前筛选器必须关闭。
  if (pending.setOverlay.touchesOverlayConfig()) {
    if (snapshot.current.filter.enabled) {
      journal.recordFailure(ApplicationText::tr("✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first."));
    } else {
      try {
        const auto next = overlayConfig.read(provider::Session::Next);
        const core::OverlayConfig baseline{.type = next.type == provider::OverlayType::RAM ? core::OverlayType::RAM : core::OverlayType::Disk,
                                           .maximumSizeMb = next.maximumSize};
        const core::OverlayConfigDelta changes{.type = pending.setOverlay.type,
                                               .maximumSizeMb = pending.setOverlay.maximumSizeMb,
                                               .warningThresholdMb = std::nullopt,
                                               .criticalThresholdMb = std::nullopt};
        auto previous = ApplyJournal::StepResult::Confirmed;
        for (const auto& step : core::planOverlayWrites(baseline, changes)) {
          const bool ready = !step.requiresPrevious || previous == ApplyJournal::StepResult::Confirmed;
          if (step.field == core::OverlayField::Type) {
            const auto type = *changes.type;
            journal.begin(ConfigurationOperationKind::OverlayType);
            const char* tStr = type == core::OverlayType::RAM ? "RAM" : "Disk";
            previous = journal.apply(
                {.success = ApplicationText::tr("✓ Overlay type set to %1").arg(tStr), .failure = ApplicationText::tr("✘ Failed to set overlay type")},
                [&] { overlayConfig.setType(next, coreTypeToProvider(type)); }, ready);
          } else {
            const auto value = *changes.maximumSizeMb;
            journal.begin(ConfigurationOperationKind::OverlaySize);
            if (changes.type.value_or(baseline.type) == core::OverlayType::Disk && value < core::kDiskOverlayMinSizeMb) {
              journal.recordFailure(
                  ApplicationText::tr("✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.").arg(core::kDiskOverlayMinSizeMb));
              previous = ApplyJournal::StepResult::Failed;
            } else {
              previous = journal.apply(
                  {.success = ApplicationText::tr("✓ Overlay maximum size set to %1 MB").arg(value),
                   .failure = ApplicationText::tr("✘ Failed to set maximum size")},
                  [&] { overlayConfig.setMaximumSize(next, value); }, ready);
            }
          }
        }
      } catch (const std::exception& error) {
        journal.recordError(ApplicationText::tr("✘ Failed to read overlay configuration"), error);
      }
    }
  }

  if (!pending.volumeProtect.empty() || !pending.volumeBindByVolumeName.empty() || !pending.addFileExclusions.empty() ||
      !pending.removeFileExclusions.empty()) {
    try {
      auto volumes = volume.readAll();
      std::set<std::string> registrationFailures;
      // 缺少 next-session 实例时从 current 复制卷标识，并缓存新行供同批次操作复用。
      auto getOrCreateNextVolume = [&](const std::string& dl) -> std::optional<provider::VolumeRow> {
        if (const auto* hit = findNextVolume(volumes, dl)) return *hit;
        // 本批次不重试失败或未确认的卷注册；后续操作跳过该卷，等待宿主对账。
        if (registrationFailures.contains(dl)) return std::nullopt;
        journal.begin(ConfigurationOperationKind::VolumeRegistration, dl);
        try {
          auto registration = volume.ensureNextSessionEntry(dl);
          journal.registrationObserved(registration.disposition, dl);
          volumes.push_back(registration.row);
          return registration.row;
        } catch (const std::exception& error) {
          registrationFailures.insert(dl);
          journal.recordError(ApplicationText::tr("✘ Volume %1: failed to register with UWF").arg(QString::fromStdString(dl)), error);
        }
        return std::nullopt;
      };

      for (const auto& [dl, wantProtect] : pending.volumeProtect) {
        auto v = getOrCreateNextVolume(dl);
        if (!v) continue;
        journal.begin(ConfigurationOperationKind::VolumeProtection, dl);
        journal.apply({.success = ApplicationText::tr("✓ Volume %1 protection: %2")
                                      .arg(QString::fromStdString(dl), wantProtect ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")),
                       .failure = ApplicationText::tr("✘ Failed to %1 protection on volume %2")
                                      .arg(wantProtect ? ApplicationText::tr("enable") : ApplicationText::tr("disable"), QString::fromStdString(dl))},
                      [&] {
                        if (wantProtect)
                          volume.protectVolume(*v);
                        else
                          volume.unprotect(*v);
                      });
      }

      for (const auto& [dl, byVolumeName] : pending.volumeBindByVolumeName) {
        auto v = getOrCreateNextVolume(dl);
        if (!v) continue;
        journal.begin(ConfigurationOperationKind::VolumeBinding, dl);
        const auto binding = byVolumeName ? provider::VolumeBinding::VolumeName : provider::VolumeBinding::DriveLetter;
        journal.apply({.success = ApplicationText::tr("✓ Volume %1 bind by: %2")
                                      .arg(QString::fromStdString(dl), byVolumeName ? ApplicationText::tr("volume ID") : ApplicationText::tr("drive letter")),
                       .failure = ApplicationText::tr("✘ Failed to set binding for volume %1").arg(QString::fromStdString(dl))},
                      [&] { volume.setBinding(*v, binding); });
      }

      for (const auto& [dl, paths] : pending.addFileExclusions) {
        if (paths.empty()) continue;
        auto v = getOrCreateNextVolume(dl);
        if (!v) continue;
        for (const auto& path : paths) {
          journal.begin(ConfigurationOperationKind::AddFileExclusion, dl + path);
          journal.apply(
              {.success = ApplicationText::tr("✓ Volume %1 added file exclusion: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)),
               .failure = ApplicationText::tr("✘ Volume %1 failed to add file exclusion %2").arg(QString::fromStdString(dl), QString::fromStdString(path))},
              [&] { volume.addExclusion(*v, path); });
        }
      }
      for (const auto& [dl, paths] : pending.removeFileExclusions) {
        if (paths.empty()) continue;
        // 删除不存在的排除项不应创建 next-session 卷实例。
        const auto* v = findNextVolume(volumes, dl);
        if (!v) {
          // 已确认 next-session 实例不存在，删除目标已满足；仍须刷新宿主以清除待应用状态。
          journal.requireReconciliation();
          for (const auto& path : paths) {
            journal.begin(ConfigurationOperationKind::RemoveFileExclusion, dl + path);
            journal.recordSuccess(
                ApplicationText::tr("✓ Volume %1 file exclusion already absent: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)));
          }
          continue;
        }
        for (const auto& path : paths) {
          journal.begin(ConfigurationOperationKind::RemoveFileExclusion, dl + path);
          journal.apply(
              {.success = ApplicationText::tr("✓ Volume %1 removed file exclusion: %2").arg(QString::fromStdString(dl), QString::fromStdString(path)),
               .failure = ApplicationText::tr("✘ Volume %1 failed to remove file exclusion %2").arg(QString::fromStdString(dl), QString::fromStdString(path))},
              [&] { volume.removeExclusion(*v, path); });
        }
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read volume configuration"), error);
    }
  }

  journal.endOperation();
  if (!pending.addRegistryExclusions.empty() || !pending.removeRegistryExclusions.empty() || pending.setPersistDomainSecretKey || pending.setPersistTSCAL) {
    try {
      const auto next = registry.read(provider::Session::Next);
      for (const auto& k : pending.addRegistryExclusions) {
        journal.begin(ConfigurationOperationKind::AddRegistryExclusion, k);
        journal.apply({.success = ApplicationText::tr("✓ Added registry exclusion: %1").arg(QString::fromStdString(k)),
                       .failure = ApplicationText::tr("✘ Failed to add registry exclusion %1").arg(QString::fromStdString(k))},
                      [&] { registry.addExclusion(next, k); });
      }
      for (const auto& k : pending.removeRegistryExclusions) {
        journal.begin(ConfigurationOperationKind::RemoveRegistryExclusion, k);
        journal.apply({.success = ApplicationText::tr("✓ Removed registry exclusion: %1").arg(QString::fromStdString(k)),
                       .failure = ApplicationText::tr("✘ Failed to remove registry exclusion %1").arg(QString::fromStdString(k))},
                      [&] { registry.removeExclusion(next, k); });
      }
      // 整实例写入时保留 next 行中未修改的持久化开关。
      if (pending.setPersistDomainSecretKey || pending.setPersistTSCAL) {
        // One provider write covers both requested switches. A failure belongs
        // to each affected operation, with the same observed write effect.
        journal.endOperation();
        if (pending.setPersistDomainSecretKey) journal.include(ConfigurationOperationKind::DomainPersistence);
        if (pending.setPersistTSCAL) journal.include(ConfigurationOperationKind::LicensePersistence);
        const provider::RegistryPersistence persistence{.domainSecretKey = pending.setPersistDomainSecretKey.value_or(next.persistDomainSecretKey),
                                                        .terminalServicesClientAccessLicense = pending.setPersistTSCAL.value_or(next.persistTSCAL)};
        if (journal.execute(ApplicationText::tr("✘ Failed to update registry persistence switches"), [&] { registry.setPersistence(next, persistence); }) ==
            ApplyJournal::StepResult::Confirmed) {
          if (pending.setPersistDomainSecretKey) {
            journal.begin(ConfigurationOperationKind::DomainPersistence);
            journal.confirmed();
            journal.recordSuccess(ApplicationText::tr("✓ %1 persistence: %2")
                                      .arg(ApplicationText::tr("Domain Secret Key (DomainSecretKey)"),
                                           *pending.setPersistDomainSecretKey ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")));
          }
          if (pending.setPersistTSCAL) {
            journal.begin(ConfigurationOperationKind::LicensePersistence);
            journal.confirmed();
            journal.recordSuccess(ApplicationText::tr("✓ %1 persistence: %2")
                                      .arg(ApplicationText::tr("Terminal Services Client Access License (TSCAL)"),
                                           *pending.setPersistTSCAL ? ApplicationText::tr("Enabled") : ApplicationText::tr("Disabled")));
          }
        }
      }
    } catch (const std::exception& error) {
      journal.recordError(ApplicationText::tr("✘ Failed to read registry filter"), error);
    }
  }

  // Storage completion and future-session coverage are separate facts. Only
  // the plan's actual durability dependencies can retain staging repair intent.
  if (stagingResult && !stagingResult->failure && !journal.satisfiesStagingCompletion(plan))
    stagingResult->failure = std::make_exception_ptr(OperationError(
        std::make_error_code(std::errc::state_not_recoverable), ApplicationText::tr("File staging could not be saved").toStdString(), WriteEffect::Uncertain));

  return {journal.lines(),
          journal.anyWriteConfirmed(),
          journal.reconciliationRequired(),
          true,
          journal.succeededOperations(),
          journal.failedOperations(),
          journal.steps(plan),
          stagingResult};
}

}  // namespace
ConfigurationActions::ApplyOperation ConfigurationExecutor::prepareApply(const core::PendingChanges& changes, const core::UwfSnapshot& snapshot) {
  auto plan = planConfiguration(changes, snapshot, m_stagingChanges);
  auto write = m_staging && plan.staging ? m_staging->prepareWrite(*plan.staging) : FileStagingWriteOperation{};
  return [session = m_session, mounts = m_mounts, plan = std::move(plan), write = std::move(write)] { return executePlan(session(), plan, write, mounts); };
}
void ConfigurationExecutor::completeApply(const ConfigurationApplyResult& result) {
  if (m_staging && result.staging) m_staging->acceptWrite(*result.staging);
}
}  // namespace uwf::app
