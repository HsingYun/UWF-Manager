// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 导入 CLI 文本或默认规则，通过注入回调更新待应用状态，累计显示逐行结果。
// File loading uses the injected picker; importing only updates pending configuration.

#include <QList>
#include <QString>
#include <array>
#include <functional>
#include <memory>

#include "app/configuration/UwfmgrCommands.h"
#include "presentation/configuration/ImportViewModel.h"
#include "ui/dialogs/PanelDialog.h"

class QPushButton;
class QTableView;
class QLabel;
class QTabWidget;

namespace uwf::ui {
class CodeEditor;
class TextLabel;

namespace dialogs {
class FileDialogProvider;
}

class ImportDialog : public PanelDialog {
  Q_OBJECT
 public:
  ImportDialog(std::shared_ptr<presentation::ImportViewModel> model, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);

  // 命令按输入顺序更新编辑上下文；系统写入仍由应用计划执行。
  using Applier = presentation::ImportViewModel::Applier;
  void setApplier(Applier a) { m_viewModel->setApplier(std::move(a)); }
  [[nodiscard]] bool reviewRequested() const { return m_reviewRequested; }

 private slots:
  void onImportClicked();

 private:
  // 追加本批结果并累计统计，第二批起保留分隔行。
  void renderReport(const QList<app::ImportResult>& rows);
  void renderActions();
  void showResult(const QModelIndex& index);

  std::shared_ptr<presentation::ImportViewModel> m_viewModel;
  dialogs::FileDialogProvider& m_fileDialogs;
  CodeEditor* m_text = nullptr;
  QTableView* m_report = nullptr;
  QWidget* m_summary = nullptr;
  std::array<TextLabel*, 4> m_summaryLabels{};
  QPushButton* m_importBtn = nullptr;
  QPushButton* m_closeBtn = nullptr;
  QTabWidget* m_tabs = nullptr;
  bool m_reviewRequested = false;
};

}  // namespace uwf::ui
