// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QHash>
#include <QPointer>
#include <QString>
#include <QWidget>
#include <memory>
#include <optional>
#include <string>

#include "application/configuration/exclusions/ExclusionDraft.h"
#include "domain/uwf/model/UwfModel.h"
#include "presentation/viewmodels/common/ViewModelFactory.h"
#include "presentation/viewmodels/configuration/VolumeConfigurationViewModel.h"

class QAction;
class QLabel;
class QPushButton;
class QTabWidget;
class QStackedWidget;

namespace uwf::ui {

class VolumeStatusPanel;
class FileStagingWidget;
class ExclusionListWidget;
class TextLabel;
class Notice;
namespace dialogs {
class FileDialogProvider;
}

class VolumeTab : public QWidget {
  Q_OBJECT
 public:
  enum class InfoPage { FileExclusions, RegistryExclusions, FileStaging };
  VolumeTab(std::shared_ptr<presentation::VolumeConfigurationViewModel> model, dialogs::FileDialogProvider& fileDialogs,
            std::shared_ptr<presentation::FileStagingViewModel> stagingModel, std::shared_ptr<presentation::ViewModelFactory> viewModelFactory,
            QWidget* parent = nullptr, presentation::MainWindowViewModel* windowModel = nullptr);
  [[nodiscard]] presentation::VolumeConfigurationViewModel& viewModel() const { return *m_viewModel; }

  void setLayoutSize(const QSize& size);
  void markUnsupported() const;
  // Limited file systems retain volume controls while disabling file-specific operations.
  void markLimitedFileSystem() const;

  [[nodiscard]] QString driveLetter() const { return QString::fromStdString(m_volume.driveLetter); }
  [[nodiscard]] QString volumeIdentity() const { return m_viewModel->volumeIdentity(); }
  // Page identity survives differing tab indexes on volumes with registry controls.
  [[nodiscard]] InfoPage activeInfoPage() const;
  void setActiveInfoPage(InfoPage page);

  [[nodiscard]] bool supported() const { return m_viewModel->supportsProtection(); }

  [[nodiscard]] bool canManageExclusions() const { return m_viewModel->supportsFileOperations(); }

 signals:
  void refreshRequested();
  void selectionDetailChanged(const QString& detail);
  void statusHint(const QString& text, int msec);
  void commitFileRequested(const QString& path);
  void commitFileDeletionRequested(const QString& path);
  // nullopt selects the whole key; a present empty name selects its default value.
  void commitRegistryRequested(const QString& key, const std::optional<QString>& valueName);
  void commitRegistryDeletionRequested(const QString& key, const std::optional<QString>& valueName);
  void fullCommitRequested();

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private slots:
  void onCommitFile();
  void onCommitDir();
  void onCommitFileDelete();
  void onCommitFolderDelete();
  void onCommitRegistry();
  void onCommitRegistryDelete();

 private:
  // Target-specific capabilities come from the volume model. Registry actions
  // are presented only on the system volume's page.
  void renderCommitAvailability(bool diagnostic) const;
  void renderRuntimeNotice();
  [[nodiscard]] bool canCommitFiles() const;
  [[nodiscard]] bool canCommitRegistry() const;

  void refreshThemedIcons();
  void renderVolumeMetadata();
  void renderState();

  [[nodiscard]] std::optional<QString> diagnostic() const;
  QWidget* pageHost(QWidget* content) const;

  std::shared_ptr<presentation::ViewModelFactory> m_viewModelFactory;
  QPointer<presentation::MainWindowViewModel> m_windowModel;
  std::shared_ptr<presentation::VolumeConfigurationViewModel> m_viewModel;
  core::VolumeInfo m_volume;
  QLabel* m_headingLabel = nullptr;
  TextLabel* m_volumeLabel = nullptr;
  TextLabel* m_capacityLabel = nullptr;
  QPushButton* m_overlayBtn = nullptr;
  QPushButton* m_commitBtn = nullptr;
  QAction* m_commitFileAct = nullptr;
  QAction* m_commitDirAct = nullptr;
  QAction* m_commitFileDeleteAct = nullptr;
  QAction* m_commitFolderDeleteAct = nullptr;
  QAction* m_commitRegAct = nullptr;
  QAction* m_commitRegDeleteAct = nullptr;
  QAction* m_fullCommitAct = nullptr;
  QAction* m_removeFullCommitDraftAct = nullptr;
  QAction* m_individualCommitNoteAct = nullptr;
  TextLabel* m_individualCommitNote = nullptr;
  TextLabel* m_fullCommitNote = nullptr;
  Notice* m_runtimeNotice = nullptr;
  VolumeStatusPanel* m_status = nullptr;
  ExclusionListWidget* m_files = nullptr;
  ExclusionListWidget* m_regs = nullptr;
  FileStagingWidget* m_fileStaging = nullptr;
  QStackedWidget* m_listCommands = nullptr;
  QTabWidget* m_infoTabs = nullptr;
  QHash<QWidget*, QStackedWidget*> m_pageHosts;
  bool m_showRegistry = false;
  dialogs::FileDialogProvider& m_fileDialogs;
};

}  // namespace uwf::ui
