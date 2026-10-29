// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VolumeConfigurationViewModel.h"

#include <QStringList>

#include "presentation/common/PresentationText.h"
#include "shared/ByteFormat.h"
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
      return PresentationText::tr("%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.")
          .arg(QString::fromStdString(fileSystem));
    case core::VolumeSupport::QueryFailed:
      return PresentationText::tr("Failed to read volume information.");
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

VolumeConfigurationViewModel::CommitAvailability VolumeConfigurationViewModel::commitAvailability() const {
  CommitAvailability result;
  const auto& state = snapshotState();
  const auto& item = volume();
  const bool globalFilterOn = state.filterEnabled;
  const bool thisVolumeProtected = supportsProtection() && state.currentProtected;
  const bool hasRegistry = static_cast<bool>(m_registry);
  result.files = state.editable && globalFilterOn && thisVolumeProtected && supportsFileOperations();
  result.registry = state.editable && globalFilterOn && hasRegistry;
  const bool isNtfs = QString::fromStdString(item.fileSystem).compare("NTFS", Qt::CaseInsensitive) == 0;
  result.overlay = state.editable && isNtfs && globalFilterOn && thisVolumeProtected;
  if (!isNtfs)
    result.overlayHint = PresentationText::tr("Overlay file listing is only supported on NTFS volumes.");
  else if (!globalFilterOn)
    result.overlayHint = PresentationText::tr("The UWF filter is currently disabled, so no volume has an overlay to inspect.");
  else if (!thisVolumeProtected)
    result.overlayHint = PresentationText::tr("This volume is not currently protected by UWF, so it has no overlay to inspect.");
  else
    result.overlayHint = PresentationText::tr("Open a diagnostic view of files currently cached in the overlay for this volume.");
  if (!globalFilterOn)
    result.commitHint = PresentationText::tr("The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.");
  else if (!supportsProtection())
    result.commitHint =
        hasRegistry ? PresentationText::tr("This volume does not support file commit. Only registry commit is available (registry exclusions are global).")
                    : PresentationText::tr("This volume is not supported by UWF.");
  else if (!supportsFileOperations())
    result.commitHint =
        (hasRegistry ? PresentationText::tr(
                           "Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).")
                     : PresentationText::tr("Per-file commit is not supported on the %1 file system."))
            .arg(QString::fromStdString(item.fileSystem));
  else if (!thisVolumeProtected)
    result.commitHint =
        hasRegistry ? PresentationText::tr("This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).")
                    : PresentationText::tr("This volume is not currently protected by UWF; there are no file changes to commit.");
  else
    result.commitHint = PresentationText::tr("Commit overlay changes to disk / registry. This action cannot be undone.");
  return result;
}
VolumeConfigurationViewModel::VolumeConfigurationViewModel(core::VolumeInfo volume, bool hostsRegistry)
    : VolumeConfigurationViewModel(std::make_shared<app::VolumeConfigurationEditor>(std::move(volume), hostsRegistry)) {}
VolumeConfigurationViewModel::VolumeConfigurationViewModel(std::shared_ptr<app::VolumeConfigurationEditor> editor)
    : m_editor(std::move(editor)),
      m_status(std::make_shared<VolumeStatusViewModel>(m_editor->status())),
      m_files(std::make_shared<ExclusionListViewModel>(m_editor->files())) {
  if (m_editor->registry()) m_registry = std::make_shared<ExclusionListViewModel>(m_editor->registry());
  connect(m_editor.get(), &app::VolumeConfigurationEditor::stateChanged, this, &VolumeConfigurationViewModel::stateChanged);
  connect(m_editor.get(), &app::VolumeConfigurationEditor::pendingChanged, this, &VolumeConfigurationViewModel::pendingChanged);
}
}  // namespace uwf::presentation

#include "moc_VolumeConfigurationViewModel.cpp"
