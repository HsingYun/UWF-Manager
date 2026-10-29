// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ApplyPlanViewModel.h"

#include <QLocale>
#include <QPointer>
#include <QStringList>
#include <algorithm>
#include <utility>

#include "app/staging/FileStagingRegistry.h"
#include "presentation/common/PresentationText.h"
#include "presentation/common/TextFileExport.h"

namespace uwf::presentation {
ApplyPlanViewModel::ApplyPlanViewModel(core::PendingChanges changes, core::UwfSnapshot snapshot, std::shared_ptr<app::ConfigurationActions> actions,
                                       std::optional<app::FileStagingChanges> stagingChanges, app::TextFileWriter textFileWriter)
    : m_plan(app::planConfiguration(std::move(changes), std::move(snapshot), std::move(stagingChanges))),
      m_actions(std::move(actions)),
      m_textFileWriter(std::move(textFileWriter)) {
  m_changeCommandCount = app::renderCommandLines(app::renderPendingChanges(m_plan.changes, m_plan.snapshot.next.overlay)).size();
  if (m_plan.changes.setOverlay.touchesOverlayConfig() && m_plan.snapshot.current.filter.enabled)
    m_warning = PresentationText::tr("Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.");
  m_exportCommands = app::renderPlannedSession(m_plan.snapshot.next, m_plan.changes);
  m_canApply = changeCount() > 0;
  const auto enabledText = [](const bool value) { return value ? PresentationText::tr("On") : PresentationText::tr("Off"); };
  const auto typeText = [](const core::OverlayType type) {
    return type == core::OverlayType::RAM    ? QStringLiteral("RAM")
           : type == core::OverlayType::Disk ? PresentationText::tr("Disk")
                                             : PresentationText::tr("Unknown");
  };
  const auto sizeText = [](const uint32_t value) { return QLocale(QLocale::English, QLocale::UnitedStates).toString(value) + QStringLiteral(" MB"); };
  ChangeGroup protection{PresentationText::tr("Protection and overlay"), QStringLiteral(":/icons/enhanced.svg"), {}};
  const auto& baseline = m_plan.snapshot.next;
  if (m_plan.changes.setFilterEnabled)
    protection.items.append({PresentationText::tr("Write protection"), PresentationText::tr("UWF filter"), enabledText(baseline.filter.enabled),
                             enabledText(*m_plan.changes.setFilterEnabled)});
  const auto& overlay = m_plan.changes.setOverlay;
  if (overlay.type)
    protection.items.append({PresentationText::tr("Overlay type"), PresentationText::tr("Storage"), typeText(baseline.overlay.type), typeText(*overlay.type)});
  if (overlay.maximumSizeMb)
    protection.items.append({PresentationText::tr("Overlay maximum size"), typeText(overlay.type.value_or(baseline.overlay.type)),
                             sizeText(baseline.overlay.maximumSizeMb), sizeText(*overlay.maximumSizeMb)});
  if (overlay.warningThresholdMb)
    protection.items.append({PresentationText::tr("Warning threshold"), PresentationText::tr("Overlay usage"), sizeText(baseline.overlay.warningThresholdMb),
                             sizeText(*overlay.warningThresholdMb)});
  if (overlay.criticalThresholdMb)
    protection.items.append({PresentationText::tr("Critical threshold"), PresentationText::tr("Overlay usage"), sizeText(baseline.overlay.criticalThresholdMb),
                             sizeText(*overlay.criticalThresholdMb)});
  for (const auto& [drive, protect] : m_plan.changes.volumeProtect) {
    const auto found = std::ranges::find_if(baseline.volumes, [&](const auto& volume) { return volume.driveLetter == drive; });
    protection.items.append({PresentationText::tr("Volume %1 protection").arg(QString::fromStdString(drive)), PresentationText::tr("Write protection"),
                             found == baseline.volumes.end() ? PresentationText::tr("Unknown") : enabledText(found->isProtected), enabledText(protect)});
  }
  for (const auto& [drive, byVolume] : m_plan.changes.volumeBindByVolumeName) {
    const auto binding = [](const bool value) { return value ? PresentationText::tr("By volume ID") : PresentationText::tr("By drive letter"); };
    const auto found = std::ranges::find_if(baseline.volumes, [&](const auto& volume) { return volume.driveLetter == drive; });
    protection.items.append({PresentationText::tr("Volume %1 binding").arg(QString::fromStdString(drive)), PresentationText::tr("Volume identity"),
                             found == baseline.volumes.end() ? PresentationText::tr("Unknown") : binding(!found->bindByDriveLetter), binding(byVolume)});
  }
  if (!protection.items.isEmpty()) m_groups.append(std::move(protection));
  ChangeGroup files{PresentationText::tr("File exclusions"), QStringLiteral(":/icons/folder.svg"), {}};
  const auto appendPaths = [](ChangeGroup& group, const auto& paths, const bool added, const QString& description) {
    for (const auto& path : paths)
      group.items.append({QString::fromStdString(path), description, {}, added ? PresentationText::tr("Added") : PresentationText::tr("Removed"), true});
  };
  for (const auto& [drive, paths] : m_plan.changes.addFileExclusions) {
    Q_UNUSED(drive);
    appendPaths(files, paths, true, PresentationText::tr("Future writes to this path will not be protected by UWF."));
  }
  for (const auto& [drive, paths] : m_plan.changes.removeFileExclusions) {
    Q_UNUSED(drive);
    appendPaths(files, paths, false, PresentationText::tr("Remove this path from the next-session exclusions."));
  }
  if (!files.items.isEmpty()) m_groups.append(std::move(files));
  ChangeGroup registry{PresentationText::tr("Registry exclusions"), QStringLiteral(":/icons/registry.svg"), {}};
  for (const auto& path : m_plan.changes.addRegistryExclusions) {
    const auto key = QString::fromStdString(path);
    const bool stagingDependency = m_plan.staging && !m_plan.staging->after.isEmpty() && app::isFileStagingRegistryRoot(key);
    registry.items.append({key,
                           stagingDependency ? PresentationText::tr("File staging relies on this registry exclusion to preserve its list across restarts.")
                                             : PresentationText::tr("Future writes to this key will not be protected by UWF."),
                           {}, stagingDependency ? PresentationText::tr("Built-in") : PresentationText::tr("Added"), true});
  }
  appendPaths(registry, m_plan.changes.removeRegistryExclusions, false, PresentationText::tr("Remove this key from the next-session exclusions."));
  if (m_plan.changes.setPersistDomainSecretKey)
    registry.items.append({PresentationText::tr("Domain Secret Key persistence"),
                           {},
                           enabledText(baseline.persistDomainSecretKey),
                           enabledText(*m_plan.changes.setPersistDomainSecretKey)});
  if (m_plan.changes.setPersistTSCAL)
    registry.items.append(
        {PresentationText::tr("Terminal Services license persistence"), {}, enabledText(baseline.persistTSCAL), enabledText(*m_plan.changes.setPersistTSCAL)});
  if (!registry.items.isEmpty()) m_groups.append(std::move(registry));
  if (m_plan.staging) {
    ChangeGroup staging{PresentationText::tr("File staging"), QStringLiteral(":/icons/file_staging.svg"), {}};
    if (m_plan.staging->persistencePending && m_plan.staging->added().isEmpty() && m_plan.staging->removed().isEmpty())
      staging.items.append({PresentationText::tr("File staging"), PresentationText::tr("File staging is saved after Apply"),
                            PresentationText::tr("Unavailable"), PresentationText::tr("Ready")});
    for (const auto entries = m_plan.staging->removed(); const auto& entry : entries)
      staging.items.append({entry.path, PresentationText::tr("Remove from automatic file staging after Apply."), {}, PresentationText::tr("Removed"), true});
    for (const auto entries = m_plan.staging->added(); const auto& entry : entries)
      staging.items.append({entry.path, PresentationText::tr("Add to automatic file staging after Apply."), {}, PresentationText::tr("Added"), true});
    m_groups.append(std::move(staging));
    if (m_plan.staging->after.isEmpty()) {
      if (!m_warning.isEmpty()) m_warning += u'\n';
      m_warning += PresentationText::tr("Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.");
    }
  }
  for (int group = 0; group < m_groups.size(); ++group) {
    m_rows.append({group, -1});
    for (int entry = 0; entry < m_groups[group].items.size(); ++entry) m_rows.append({group, entry});
  }
}

int ApplyPlanViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : static_cast<int>(m_rows.size()) + 1; }
QVariant ApplyPlanViewModel::data(const QModelIndex& index, const int role) const {
  if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= rowCount()) return {};
  if (index.row() == m_rows.size()) return role == KindRole ? QVariant(Commands) : QVariant{};
  const auto& row = m_rows[index.row()];
  const auto& group = m_groups[row.group];
  if (role == KindRole) return row.entry < 0 ? Heading : Entry;
  if (row.entry < 0) {
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) return group.title;
    if (role == Qt::DecorationRole) return group.icon;
    return {};
  }
  const auto& change = group.items[row.entry];
  switch (role) {
    case Qt::DisplayRole:
      return change.title;
    case Qt::AccessibleTextRole:
      return change.title + u'\n' + change.description + u'\n' + change.before + u'\n' + change.after;
    case DescriptionRole:
      return change.description;
    case BeforeRole:
      return change.before;
    case AfterRole:
      return change.after;
    case PathRole:
      return change.path;
    case LastRole:
      return row.entry + 1 == group.items.size();
    default:
      return {};
  }
}
Qt::ItemFlags ApplyPlanViewModel::flags(const QModelIndex& index) const { return index.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags; }

QString ApplyPlanViewModel::changeCommandsText() const {
  return QString::fromStdString(app::renderCommandScript(app::renderPendingChanges(m_plan.changes, m_plan.snapshot.next.overlay)));
}
QString ApplyPlanViewModel::targetCommandsText() const {
  return QString::fromStdString(app::renderCommandScript(m_exportCommands));
}
QString ApplyPlanViewModel::resultText() const {
  QStringList lines;
  for (const auto& line : m_result.lines) lines.append(QString::fromStdString(line));
  return lines.join(u'\n');
}

bool ApplyPlanViewModel::resultFailed() const { return !m_result.connectionEstablished || m_result.failedOperations > 0; }

bool ApplyPlanViewModel::canRestart() const { return m_result.connectionEstablished && m_result.anyWriteConfirmed; }

QString ApplyPlanViewModel::resultHeading() const {
  return resultFailed() ? PresentationText::tr("Some changes could not be applied") : PresentationText::tr("Configuration applied");
}

QString ApplyPlanViewModel::resultDescription() const {
  return resultFailed()
             ? PresentationText::tr("Review the operation details. Read the current system state before retrying changes that may have been written.")
             : PresentationText::tr("The configuration has been written to the system. Most changes take effect after restart.");
}

void ApplyPlanViewModel::setExecutionAllowed(const bool allowed) {
  if (m_executionAllowed == allowed) return;
  m_executionAllowed = allowed;
  emit availabilityChanged();
}
void ApplyPlanViewModel::apply() {
  if (!canApply()) return;
  m_canApply = false;
  m_applying = true;
  const QPointer guard(this);
  emit applyingChanged();
  if (!guard) return;
  emit availabilityChanged();
  if (!guard) return;
  app::ConfigurationActions::ApplyOperation operation;
  try {
    operation = m_actions->prepareApply(m_plan.changes, m_plan.snapshot);
  } catch (const std::exception& error) {
    app::ConfigurationApplyResult result;
    result.failedOperations = 1;
    result.lines.push_back(error.what());
    publishResult(std::move(result));
    return;
  }
  m_apply.start([operation = std::move(operation)](std::stop_token) { return operation(); },
                [this](AsyncResult<app::ConfigurationApplyResult> result) {
                  if (auto* value = std::get_if<app::ConfigurationApplyResult>(&result)) {
                    publishResult(std::move(*value));
                  } else {
                    app::ConfigurationApplyResult failure;
                    failure.reconciliationRequired = true;
                    failure.failedOperations = 1;
                    try {
                      std::rethrow_exception(std::get<std::exception_ptr>(result));
                    } catch (const app::OperationError& error) {
                      failure.reconciliationRequired = error.writeEffect() != app::WriteEffect::None;
                      failure.lines.push_back(error.what());
                    } catch (const std::exception& error) {
                      failure.lines.push_back(error.what());
                    } catch (...) {
                      failure.lines.push_back(PresentationText::tr(": unknown error").toStdString());
                    }
                    publishResult(std::move(failure));
                  }
                });
}
void ApplyPlanViewModel::publishResult(app::ConfigurationApplyResult result) {
  const QPointer guard(this);
  m_actions->completeApply(result);
  if (!guard) return;
  m_result = std::move(result);
  m_canApply = !m_result.connectionEstablished || !m_result.reconciliationRequired;
  // Install the reconciliation gate before releasing the write gate.
  if (m_result.reconciliationRequired) emit reconciliationRequired();
  if (!guard) return;
  m_applying = false;
  emit resultChanged();
  if (!guard) return;
  emit applyingChanged();
  if (!guard) return;
  emit availabilityChanged();
}

QString ApplyPlanViewModel::exportFile(const QString& path) const {
  if (!canExport()) return PresentationText::tr("Export failed");
  return exportTextFile(path, targetCommandsText(), m_textFileWriter);
}
}  // namespace uwf::presentation

#include "moc_ApplyPlanViewModel.cpp"
