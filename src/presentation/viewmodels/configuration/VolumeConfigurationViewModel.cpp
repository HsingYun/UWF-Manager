// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"

#include <QPointer>
#include <QStringList>
#include <utility>

#include "domain/uwf/policies/IndividualCommitPolicy.h"
#include "application/localization/ConfigurationModeText.h"
#include "foundation/text/ByteFormat.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
QString volumeSupportText(core::VolumeSupport s, const std::string& fileSystem) {
  switch (s) {
    case core::VolumeSupport::Supported:
      return {};
    case core::VolumeSupport::NotFixedLocalDisk:
      return PresentationText::tr("Unsupported drive type (only fixed local disks are supported).");
    case core::VolumeSupport::ExceedsMaxSize:
      return PresentationText::tr("Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.");
    case core::VolumeSupport::FileSystemLimited:
      // File-system capabilities do not establish physical registration
      // eligibility; volumeRegistrationText owns that independent decision.
      return PresentationText::tr("%1 file system: file exclusions and per-file commit are not supported.")
          .arg(QString::fromStdString(fileSystem));
  }
  return {};
}

QString volumeRegistrationText(const core::VolumeInfo& volume) {
  if (!core::supportsVolumeProtection(volume.support)) return volumeSupportText(volume.support, volume.fileSystem);
  if (core::canRegisterVolume(volume)) return {};
  switch (core::physicalVolumeAdmission(volume.physical)) {
    case core::PhysicalVolumeAdmission::NotFixedMedia:
      return PresentationText::tr("Unsupported drive type (only fixed local disks are supported).");
    case core::PhysicalVolumeAdmission::UnsupportedBus:
      return PresentationText::tr("This volume cannot be registered with UWF because its storage bus is not supported.");
    case core::PhysicalVolumeAdmission::Unknown:
    case core::PhysicalVolumeAdmission::Eligible:
      return PresentationText::tr("Physical volume support could not be confirmed. Refresh before enabling protection.");
  }
  return {};
}

QString VolumeConfigurationViewModel::capacityText() const {
  const auto& item = volume();
  QStringList parts;
  if (!item.fileSystem.empty()) parts << QString::fromStdString(item.fileSystem);
  if (item.totalBytes > 0)
    parts << PresentationText::tr("%1 free / %2")
                 .arg(QString::fromStdString(formatBytes(item.freeBytes)).replace(QStringLiteral(".00 "), QStringLiteral(" ")),
                      QString::fromStdString(formatBytes(item.totalBytes)).replace(QStringLiteral(".00 "), QStringLiteral(" ")));
  return parts.join(QStringLiteral(" · "));
}

bool VolumeConfigurationViewModel::currentReadOnlyMediaActive() const {
  const auto& state = snapshotState();
  return state.available && state.filterEnabled && state.currentProtected &&
         core::effectiveReadOnlyMedia({state.hormEnabled, state.currentOverlayFlags, state.romDisposition, state.fullCommit}) == true;
}

bool VolumeConfigurationViewModel::currentHormActive() const { return snapshotState().available && snapshotState().hormEnabled == true; }

QString VolumeConfigurationViewModel::runtimeNotice() const {
  if (currentHormActive())
    return PresentationText::tr("HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.");
  if (snapshotState().configurationRestriction == core::ConfigurationWriteRestriction::RomVolatile)
    return app::configurationModeRestrictionText(snapshotState().configurationRestriction);
  if (currentReadOnlyMediaActive())
    return PresentationText::tr(
        "Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume "
        "commit affects all protected volumes; review its description before continuing.");
  if (snapshotState().configurationRestriction != core::ConfigurationWriteRestriction::None)
    return app::configurationModeRestrictionText(snapshotState().configurationRestriction);
  if (!m_status->draft().hasConfiguration && core::supportsVolumeProtection(volume().support)) return volumeRegistrationText(volume());
  return {};
}

OverlayMaintenanceViewModel::RuntimeNotice VolumeConfigurationViewModel::runtimePresentation(const OverlayMaintenanceViewModel* maintenance) const {
  using Tone = OverlayMaintenanceViewModel::NoticeTone;
  if (maintenance)
    if (const auto notice = maintenance->runtimeNotice()) return *notice;
  OverlayMaintenanceViewModel::RuntimeNotice notice;
  notice.text = runtimeNotice();
  if (currentHormActive())
    notice.heading = QStringLiteral("HORM · ") + PresentationText::tr("Enabled");
  else if (!notice.text.isEmpty()) {
    if (currentReadOnlyMediaActive())
      notice.heading = PresentationText::tr("Read-Only Media mode is active");
    else
      notice.tone = Tone::Warning;
  }
  return notice;
}

void VolumeConfigurationViewModel::updateRuntimePresentation() {
  const auto generation = ++m_presentationGeneration;
  const auto files = m_files;
  const auto registry = m_registry;
  const bool exclusionsEffective = !currentReadOnlyMediaActive() && !currentHormActive();
  const auto commits = commitAvailability();
  const QPointer self(this);
  const auto current = [&] { return self && m_presentationGeneration == generation; };
  files->setCurrentSessionExclusionsEffective(exclusionsEffective);
  if (!current()) return;
  if (registry) registry->setCurrentSessionExclusionsEffective(exclusionsEffective);
  if (!current()) return;
  files->setCommitAvailability(commits.files, commits.commitHint);
  if (!current()) return;
  if (registry) registry->setCommitAvailability(commits.registry, commits.commitHint);
  if (current()) emit stateChanged();
}

void VolumeConfigurationViewModel::setCommitContext(const bool allowed, QString restriction) {
  if (allowed) restriction.clear();
  if (m_commitsAllowed == allowed && m_commitRestriction == restriction) return;
  m_commitsAllowed = allowed;
  m_commitRestriction = std::move(restriction);
  updateRuntimePresentation();
}

VolumeConfigurationViewModel::CommitAvailability VolumeConfigurationViewModel::commitAvailability(const OverlayMaintenanceViewModel* maintenance,
                                                                                                  const bool diagnostic) const {
  CommitAvailability result;
  result.fullCommit = maintenance ? maintenance->fullCommitState() : OverlayMaintenanceViewModel::FullCommitState{};
  if (!maintenance) {
    result.fullCommit.label = PresentationText::tr("Full volume commit");
    result.fullCommit.hint = result.fullCommit.note = PresentationText::tr("Read the current UWF state first.");
  }
  result.removeFullCommitDraft = result.fullCommit.pending;
  const auto complete = [&] {
    if (diagnostic) {
      result.files = result.registry = result.overlay = result.fullCommit.available = result.removeFullCommitDraft = false;
      result.commitHint = result.overlayHint = result.fullCommit.hint = result.fullCommit.note = PresentationText::tr("UWF is currently unavailable");
    }
    result.menu = result.files || result.registry || result.fullCommit.available || result.removeFullCommitDraft;
    return result;
  };
  if (driveLetter().isEmpty()) {
    result.commitHint = result.overlayHint = PresentationText::tr("Read-only");
    return complete();
  }
  const auto& state = snapshotState();
  const auto& item = volume();
  const bool globalFilterOn = state.filterEnabled;
  const bool thisVolumeProtected = supportsProtection() && state.currentProtected;
  const bool hasRegistry = static_cast<bool>(m_registry);
  const core::IndividualCommitState commitState{state.hormEnabled, state.currentOverlayFlags, state.romDisposition, state.fullCommit};
  const auto restriction = core::individualCommitRestriction(globalFilterOn, thisVolumeProtected, commitState);
  // Execution rejects known restrictions and retains the provider's final
  // checks. The UI must additionally have evidence that its mode permits commit.
  const auto effectiveRom = core::effectiveReadOnlyMedia(commitState);
  const bool commitsAllowed = restriction == core::IndividualCommitRestriction::None && effectiveRom == false;
  result.files = m_commitsAllowed && m_editor->maintenanceEditingAllowed() && globalFilterOn && thisVolumeProtected && supportsFileOperations() && commitsAllowed;
  result.registry = m_commitsAllowed && m_editor->maintenanceEditingAllowed() && globalFilterOn && thisVolumeProtected && hasRegistry && commitsAllowed;
  const bool isNtfs = QString::fromStdString(item.fileSystem).compare("NTFS", Qt::CaseInsensitive) == 0;
  result.overlay = state.available && state.elevated && isNtfs && globalFilterOn && thisVolumeProtected;
  if (!state.available)
    result.overlayHint = PresentationText::tr("UWF is currently unavailable");
  else if (!state.elevated)
    result.overlayHint = PresentationText::tr("Read-only");
  else if (!isNtfs)
    result.overlayHint = PresentationText::tr("Overlay file listing is only supported on NTFS volumes.");
  else if (!globalFilterOn)
    result.overlayHint = PresentationText::tr("The UWF filter is disabled; no volume has an overlay to inspect.");
  else if (!thisVolumeProtected)
    result.overlayHint = PresentationText::tr("This volume is not currently protected by UWF, so it has no overlay to inspect.");
  else
    result.overlayHint = PresentationText::tr("Open a diagnostic view of files currently cached in the overlay for this volume.");
  if (!state.available)
    result.commitHint = PresentationText::tr("UWF is currently unavailable");
  else if (!state.elevated)
    result.commitHint = PresentationText::tr("Read-only");
  else if (!globalFilterOn)
    result.commitHint = PresentationText::tr("The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.");
  else if (hasRegistry && !thisVolumeProtected)
    result.commitHint = PresentationText::tr("The system volume is not currently protected by UWF; there are no registry changes to commit.");
  else if (!supportsProtection())
    result.commitHint = PresentationText::tr("This volume is not supported by UWF.");
  else if (!supportsFileOperations())
    result.commitHint =
        (hasRegistry ? PresentationText::tr(
                           "Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).")
                     : PresentationText::tr("Per-file commit is not supported on the %1 file system."))
            .arg(QString::fromStdString(item.fileSystem));
  else if (!thisVolumeProtected)
    result.commitHint = PresentationText::tr("This volume is not currently protected by UWF; there are no file changes to commit.");
  else
    result.commitHint = PresentationText::tr("Commit changes from the overlay to disk or the registry. This cannot be undone.");
  if (state.available && state.elevated && (supportsFileOperations() || hasRegistry)) {
    if (restriction == core::IndividualCommitRestriction::HormActive)
      result.commitHint = PresentationText::tr("File and registry commits are unavailable while HORM is active.");
    else if (restriction == core::IndividualCommitRestriction::ReadOnlyMediaActive)
      result.commitHint = PresentationText::tr("File and registry commits are unavailable while Read-Only Media mode is active.");
    else if (globalFilterOn && thisVolumeProtected && !effectiveRom)
      result.commitHint = PresentationText::tr("The commit or HORM state is unknown. Refresh before continuing.");
  }
  if (!m_commitsAllowed)
    result.commitHint = m_commitRestriction;
  else if (state.editable && !m_editor->maintenanceEditingAllowed())
    result.commitHint = PresentationText::tr("Read-only");
  return complete();
}
VolumeConfigurationViewModel::VolumeConfigurationViewModel(core::VolumeInfo volume, bool hostsRegistry)
    : VolumeConfigurationViewModel(std::make_shared<app::VolumeConfigurationEditor>(std::move(volume), hostsRegistry)) {}
VolumeConfigurationViewModel::VolumeConfigurationViewModel(std::shared_ptr<app::VolumeConfigurationEditor> editor)
    : m_editor(std::move(editor)),
      m_status(std::make_shared<VolumeStatusViewModel>(m_editor->status())),
      m_files(std::make_shared<ExclusionListViewModel>(m_editor->files())) {
  if (m_editor->registry()) m_registry = std::make_shared<ExclusionListViewModel>(m_editor->registry());
  connect(m_editor.get(), &app::VolumeConfigurationEditor::stateChanged, this, &VolumeConfigurationViewModel::updateRuntimePresentation);
  connect(m_editor.get(), &app::VolumeConfigurationEditor::pendingChanged, this, &VolumeConfigurationViewModel::pendingChanged);
}
}  // namespace uwf::presentation

#include "moc_VolumeConfigurationViewModel.cpp"
