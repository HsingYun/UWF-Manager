// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "FileStagingProgressDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <algorithm>
#include <limits>

#include "ui/appearance/I18n.h"
#include "ui/dialogs/DialogLayout.h"
#include "ui/dialogs/PanelDialog.h"
namespace uwf::ui {
namespace {
int progressValue(const std::size_t value) { return static_cast<int>(std::min(value, static_cast<std::size_t>(std::numeric_limits<int>::max()))); }
class FileStagingProgressDialog final : public PanelDialog {
 public:
  FileStagingProgressDialog(presentation::FileStagingProgressViewModel& model, QWidget* parent) : PanelDialog(parent) {
    setObjectName(QStringLiteral("fileStagingProgressDialog"));
    setWindowFlag(Qt::WindowStaysOnTopHint);
    // Closing the progress view cannot release an unfinished shutdown barrier.
    setRejectionEnabled(false);
    setWindowTitle(I18n::tr("Committing staged files"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose, false);
    auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(480, 0), QSize(430, 0));
    auto* layout = frame->body();
    m_detail = new QLabel(I18n::tr("Calculating files for automatic commit…"), this);
    m_detail->setWordWrap(true);
    m_detail->setProperty("dialogTone", "muted");
    layout->addWidget(m_detail);

    m_progress = new QProgressBar(this);
    m_progress->setObjectName(QStringLiteral("fileStagingCommandProgress"));
    m_progress->setRange(0, 0);
    layout->addWidget(m_progress);

    m_path = new QLineEdit(this);
    m_path->setLayoutDirection(Qt::LeftToRight);
    m_path->setObjectName(QStringLiteral("fileStagingCommandPath"));
    m_path->setReadOnly(true);
    m_path->setFrame(false);
    m_path->setProperty("dialogPath", true);
    m_path->hide();
    layout->addWidget(m_path);

    connect(&model, &presentation::FileStagingProgressViewModel::presentationRequested, this, [this] {
      show();
      raise();
      activateWindow();
    });
    connect(&model, &presentation::FileStagingProgressViewModel::stateChanged, this, [this, &model] { render(model.state()); });
    connect(&model, &presentation::FileStagingProgressViewModel::presentationFinished, this, &QDialog::hide);
    connect(&model, &QObject::destroyed, this, &QObject::deleteLater);
  }

 private:
  void render(const presentation::FileStagingProgressViewModel::State& state) {
    using Phase = presentation::FileStagingProgressViewModel::Phase;
    if (state.phase == Phase::Preparing) return;
    if (state.phase == Phase::Committing) {
      m_progress->setMaximum(std::max(1, progressValue(state.total)));
      m_progress->setValue(progressValue(state.processed));
      if (!state.hasCommitProgress) return;
      m_detail->setText(I18n::tr("Processed %1 of %2 file(s).").arg(static_cast<qulonglong>(state.processed)).arg(static_cast<qulonglong>(state.total)));
      m_path->setText(state.currentPath);
      m_path->setCursorPosition(0);
      m_path->setToolTip(state.currentPath);
      m_path->setVisible(!state.currentPath.isEmpty());
      return;
    }
    m_progress->setRange(0, std::max(1, progressValue(state.total)));
    m_progress->setValue(progressValue(state.processed));
    const bool completed = state.outcome == app::ApplicationCommandOutcome::Succeeded || state.outcome == app::ApplicationCommandOutcome::CompletedWithFailures;
    setWindowTitle(completed ? I18n::tr("File staging completed") : I18n::tr("File staging failed"));
    switch (state.outcome) {
      case app::ApplicationCommandOutcome::Succeeded:
        m_detail->setText(I18n::tr("All staged targets were processed."));
        break;
      case app::ApplicationCommandOutcome::CompletedWithFailures:
        m_detail->setText(I18n::tr("All staged targets were processed; one or more operations failed."));
        break;
      case app::ApplicationCommandOutcome::Rejected:
      case app::ApplicationCommandOutcome::Failed:
        m_detail->setText(state.detail.isEmpty() ? I18n::tr("Automatic file staging stopped because of an unknown error.") : state.detail);
        break;
    }
  }
  QLabel* m_detail = nullptr;
  QProgressBar* m_progress = nullptr;
  QLineEdit* m_path = nullptr;
};
}  // namespace
void attachFileStagingProgressDialog(presentation::FileStagingProgressViewModel& model, QWidget* parent) { new FileStagingProgressDialog(model, parent); }
}  // namespace uwf::ui
