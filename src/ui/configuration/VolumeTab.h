// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QHash>
#include <QString>
#include <QWidget>
#include <memory>
#include <optional>
#include <string>

#include "app/configuration/ExclusionDraft.h"
#include "core/UwfModel.h"
#include "presentation/common/ViewModelFactory.h"
#include "presentation/configuration/VolumeConfigurationViewModel.h"

namespace uwf::app {
class FileStagingDraft;
}

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
namespace dialogs {
class FileDialogProvider;
}

class VolumeTab : public QWidget {
  Q_OBJECT
 public:
  enum class InfoPage { FileExclusions, RegistryExclusions, FileStaging };
  VolumeTab(std::shared_ptr<presentation::VolumeConfigurationViewModel> model, dialogs::FileDialogProvider& fileDialogs, app::FileStagingDraft* stagingDraft,
            std::shared_ptr<presentation::ViewModelFactory> viewModelFactory, QWidget* parent = nullptr);
  [[nodiscard]] presentation::VolumeConfigurationViewModel& viewModel() const { return *m_viewModel; }

  // UWF 不可读或进程未提权时，保护开关 / 绑定方式 / 排除列表的增删 / 提交
  // 按钮一律置灰；列表内容仍可查看、可滚动、可切换 TAB。
  void setEnhancedModeAvailable(bool available);
  void setLayoutSize(const QSize& size);
  void setDiagnostic(const QString& message);
  void markUnsupported() const;
  // 文件系统受限（exFAT 等）的 UI 处理：保护开关 / 绑定方式保持可用，
  // 但文件排除列表 readOnly + commit 文件类菜单灰掉。
  void markLimitedFileSystem() const;

  [[nodiscard]] QString driveLetter() const { return QString::fromStdString(m_volume.driveLetter); }
  // MainWindow 按业务页类型恢复选择，不能保存数值索引：注册表宿主页比其它卷
  // 多一个 TAB，相同索引可能对应不同功能。
  [[nodiscard]] InfoPage activeInfoPage() const;
  void setActiveInfoPage(InfoPage page);
  // 卷是否可被 UWF 保护——含 NTFS/FAT 完全支持 + exFAT/ReFS 等 limited
  // 支持。Limited 卷允许 protect 开关与绑定方式，但禁用文件排除 / commit。
  [[nodiscard]] bool supported() const { return m_viewModel->supportsProtection(); }
  // 卷的文件系统是否支持文件排除列表 / 单文件提交。仅 NTFS / FAT(32) 是 true。
  [[nodiscard]] bool canManageExclusions() const { return m_viewModel->supportsFileOperations(); }

 signals:
  void refreshRequested();
  void selectionDetailChanged(const QString& detail);
  void statusHint(const QString& text, int msec);
  void commitFileRequested(const QString& path);
  void commitFileDeletionRequested(const QString& path);
  // valueName 空串 = 整键递归；非空 = 操作单个命名值。(Default) 在 picker
  // 层恒禁用，故 valueName 非空时必然是真实值名，"空=递归"无歧义。
  void commitRegistryRequested(const QString& key, const QString& valueName);
  void commitRegistryDeletionRequested(const QString& key, const QString& valueName);

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
  // 持久化要求可写且当前筛选器启用；文件操作还要求本卷受保护。
  // 注册表操作是全局能力，不依赖本卷保护状态。
  void updateCommitEnablement() const;
  // 主题切换时刷新菜单图标。
  void refreshThemedIcons();
  void renderVolumeMetadata();
  void retranslateUi();

  void renderSnapshot();
  QWidget* pageHost(QWidget* content) const;

  std::shared_ptr<presentation::ViewModelFactory> m_viewModelFactory;
  std::shared_ptr<presentation::VolumeConfigurationViewModel> m_viewModel;
  core::VolumeInfo m_volume;
  QLabel* m_headingLabel = nullptr;
  TextLabel* m_volumeLabel = nullptr;
  QLabel* m_capacityLabel = nullptr;
  QPushButton* m_overlayBtn = nullptr;  // "查看覆盖层文件" 按钮（仅 NTFS）
  QPushButton* m_commitBtn = nullptr;   // "持久化" 菜单按钮
  QAction* m_commitFileAct = nullptr;
  QAction* m_commitDirAct = nullptr;
  QAction* m_commitFileDeleteAct = nullptr;
  QAction* m_commitFolderDeleteAct = nullptr;
  QAction* m_commitRegAct = nullptr;        // 仅全局注册表宿主页创建
  QAction* m_commitRegDeleteAct = nullptr;  // 仅全局注册表宿主页创建
  VolumeStatusPanel* m_status = nullptr;
  ExclusionListWidget* m_files = nullptr;
  ExclusionListWidget* m_regs = nullptr;
  FileStagingWidget* m_fileStaging = nullptr;
  QStackedWidget* m_listCommands = nullptr;
  QTabWidget* m_infoTabs = nullptr;  // 内层 TAB（文件排除 / 注册表排除 / 文件暂存）
  QHash<QWidget*, QStackedWidget*> m_pageHosts;
  bool m_showRegistry = false;
  QString m_diagnostic;
  dialogs::FileDialogProvider& m_fileDialogs;
};

}  // namespace uwf::ui
