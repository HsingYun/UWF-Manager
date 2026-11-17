// SPDX-License-Identifier: GPL-3.0-or-later
#include "../PreviewHost.h"

#include <QCoreApplication>

#include "../PreviewFixtures.h"
#include "application/diagnostics/ProcessLogSource.h"
#include "presentation/appearance/I18n.h"
#include "presentation/views/files/FilePickerDialogFactory.h"

namespace uwf::preview::host {
bool defaultInteractive() { return true; }
composition::DesktopPlatformPorts desktopPorts() {
  composition::DesktopPlatformPorts ports;
  ports.volumes = std::make_shared<PreviewVolumeRepository>(std::vector<core::VolumeInfo>{});
  ports.files = std::make_shared<PreviewFileBrowser>();
  ports.traversal = std::make_shared<PreviewCommitTraversalSource>();
  ports.registry = std::make_shared<PreviewRegistryBrowser>();
  ports.systemInformation = std::make_shared<PreviewSystemInformation>();
  ports.log = std::make_shared<app::ProcessLogSource>();
  ports.fileDialogs = ui::createFilePickerDialogs(ports.files);
  ports.applicationDirectory = QCoreApplication::applicationDirPath();
  // The picker presents simulated Windows paths, not host files. Keep text I/O
  // unavailable rather than interpreting those paths in the host filesystem.
  ports.readText = [](const QString&, qint64) {
    return app::TextFileReadResult{app::TextFileReadResult::Failure::Open,
                                   QStringLiteral("File import is unavailable in the simulated preview. Paste commands into the import dialog instead."),
                                   {}};
  };
  const auto exportError = QStringLiteral("File export is unavailable in the simulated preview.");
  ports.writeText = [exportError](const QString&, const QString&) { return app::TextFileWriteResult{app::TextFileWriteResult::Failure::Open, exportError}; };
  ports.createText = [exportError](const QString&, const QString&) {
    return app::TextFileCreationResult{app::TextFileCreationResult::Outcome::Failed, exportError};
  };
  return ports;
}
void activateForCapture(QWidget* root, QWidget*, bool) {
  root->raise();
  root->activateWindow();
}
void setCaptionStyle(QWidget*, const QString&) {}
bool isForeground(QWidget* root) { return root->isActiveWindow(); }
bool isMaximizeButton(QWidget* button) { return button->accessibleName() == ui::I18n::tr("Maximize") || button->accessibleName() == ui::I18n::tr("Restore"); }
QJsonObject materialProperties(QWidget* root) { return {{QStringLiteral("qtActive"), root->isActiveWindow()}}; }
void flushCompositor() {}
std::unique_ptr<QWidget> overlayComponent(const QString&, const app::OverlayUsageEnabled&) { return {}; }
bool taskbarPresented() { return false; }
}  // namespace uwf::preview::host
