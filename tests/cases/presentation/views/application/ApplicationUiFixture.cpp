// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "cases/presentation/views/application/ApplicationUiFixture.h"

#include <QDialogButtonBox>
#include <QTest>
#include <utility>

#include "presentation/appearance/I18n.h"

namespace uwf::test::application {
CapturingWindowWorkflows::CapturingWindowWorkflows(std::shared_ptr<ui::WindowWorkflowFactory> delegate) : m_delegate(std::move(delegate)) {}
std::unique_ptr<presentation::MainWindowViewModel> CapturingWindowWorkflows::mainWindow(std::shared_ptr<presentation::ViewModelFactory> models,
                                                                                        app::UwfCapability capability, QWidget* parent) {
  auto result = m_delegate->mainWindow(std::move(models), capability, parent);
  observed = result.get();
  return result;
}
std::unique_ptr<ui::CommitDispatcher> CapturingWindowWorkflows::commit(ui::CommitDispatcher::PauseObservation pause,
                                                                      ui::CommitDispatcher::ActivityChanged activityChanged, QWidget* parent) {
  return m_delegate->commit(std::move(pause), std::move(activityChanged), parent);
}

bool refreshCompleted(QWidget& window) {
  for (auto actions = window.actions(); auto* action : std::as_const(actions))
    if (action->shortcut() == QKeySequence(QKeySequence::Refresh)) return action->isEnabled();
  return false;
}
bool applyCompleted(QWidget& dialog) {
  for (auto boxes = dialog.findChildren<QDialogButtonBox*>(); auto* box : std::as_const(boxes))
    for (auto buttons = box->buttons(); auto* button : std::as_const(buttons))
      if (box->buttonRole(button) == QDialogButtonBox::RejectRole && button->text() == ui::I18n::tr("Close")) return button->isEnabled();
  return false;
}

void addLocaleThemeRows() {
  QTest::addColumn<ui::I18n::Lang>("locale");
  QTest::addColumn<bool>("dark");
  for (const auto& locale : ui::I18n::languages())
    for (const bool dark : {false, true})
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(QString(locale.code), dark ? "dark" : "light"))) << locale.id << dark;
}

QPushButton* buttonWithText(QWidget* root, const QString& text) {
  for (auto buttons = root->findChildren<QPushButton*>(); auto* button : std::as_const(buttons)) {
    if (button->text() == text || button->property("accessibleName").toString() == text) return button;
  }
  return nullptr;
}

QAction* actionWithText(QWidget* root, const QString& text) {
  for (auto actions = root->findChildren<QAction*>(); auto* action : std::as_const(actions)) {
    if (action->text() == text) return action;
  }
  return nullptr;
}

}  // namespace uwf::test::application
