// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QAction>
#include <QPointer>
#include <QPushButton>

#include "presentation/views/shell/WindowWorkflowFactory.h"
#include "support/app/ConfigurationFixture.h"
#include "support/app/EnhancedModeFixture.h"
#include "support/app/FileStagingFixture.h"
#include "support/ui/FileDialogStub.h"
#include "support/uwf/FakeUwfTrace.h"

namespace uwf::test::application {

using uwf::test::FileDialogStub;

class CapturingWindowWorkflows final : public ui::WindowWorkflowFactory {
 public:
  explicit CapturingWindowWorkflows(std::shared_ptr<ui::WindowWorkflowFactory> delegate);
  std::unique_ptr<presentation::MainWindowViewModel> mainWindow(std::shared_ptr<presentation::ViewModelFactory> models, app::UwfCapability capability,
                                                                QWidget* parent) override;
  std::unique_ptr<ui::CommitDispatcher> commit(ui::CommitDispatcher::PauseObservation pause, ui::CommitDispatcher::ActivityChanged activityChanged,
                                                QWidget* parent) override;
  QPointer<presentation::MainWindowViewModel> observed;

 private:
  std::shared_ptr<ui::WindowWorkflowFactory> m_delegate;
};

QPushButton* buttonWithText(QWidget* root, const QString& text);
QAction* actionWithText(QWidget* root, const QString& text);
void addLocaleThemeRows();
bool refreshCompleted(QWidget& window);
bool applyCompleted(QWidget& dialog);

}  // namespace uwf::test::application
