// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "Dialogs.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QPushButton>
#include <memory>
#include <optional>

#include "DialogLayout.h"
#include "InformationDialog.h"
#include "design/DesignTokens.h"
#include "presentation/files/FilePickerViewModel.h"
#include "ui/appearance/I18n.h"
#include "ui/dialogs/DialogAppearance.h"
#include "ui/files/FilePickerDialog.h"
#include "ui/widgets/ContentHintBox.h"

namespace uwf::ui::dialogs {

namespace {

ContentHintBox* messageText(const QString& text, QWidget* parent, const Qt::TextFormat format = Qt::PlainText) {
  auto* body = new ContentHintBox(parent);
  if (format == Qt::RichText)
    body->setHtml(text);
  else
    body->setPlainText(text);
  body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  body->setStyleSheet("QTextBrowser { background: transparent; border: 0; padding: 0; }");
  return body;
}

class SystemFileDialogProvider final : public FileDialogProvider {
  std::shared_ptr<const app::FileBrowser> m_browser;
  QStringList pick(QWidget* parent, const FileDialogRequest& request, const presentation::FilePickerViewModel::Mode mode) {
    auto model = std::make_shared<presentation::FilePickerViewModel>(mode, m_browser);
    model->setFilters(request.filter);
    model->setAllowedRoots(request.allowedRoots);
    FilePickerDialog dialog(model, request.title, parent);
    model->start(request.initialPath);
    return dialog.exec() == QDialog::Accepted ? dialog.selectedPaths() : QStringList{};
  }

 public:
  explicit SystemFileDialogProvider(std::shared_ptr<const app::FileBrowser> browser) : m_browser(std::move(browser)) {}
  QStringList openFiles(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::OpenFiles);
  }

  QString openFile(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::OpenFile).value(0);
  }

  QString selectDirectory(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::Directory).value(0);
  }

  QString saveFile(QWidget* parent, const FileDialogRequest& request) override {
    return pick(parent, request, presentation::FilePickerViewModel::Mode::SaveFile).value(0);
  }
  QVector<app::FileBrowserEntry> openItems(QWidget* parent, const FileDialogRequest& request) override {
    auto model = std::make_shared<presentation::FilePickerViewModel>(presentation::FilePickerViewModel::Mode::Mixed, m_browser);
    model->setFilters(request.filter);
    model->setAllowedRoots(request.allowedRoots);
    FilePickerDialog dialog(model, request.title, parent);
    model->start(request.initialPath);
    return dialog.exec() == QDialog::Accepted ? dialog.selectedItems() : QVector<app::FileBrowserEntry>{};
  }
};

}  // namespace

std::unique_ptr<FileDialogProvider> createFileDialogs(std::shared_ptr<const app::FileBrowser> browser) {
  return std::make_unique<SystemFileDialogProvider>(std::move(browser));
}

namespace {

struct MessageAction {
  QString text;
  DialogAction style;
};
bool runMessageDialog(QWidget* parent, const QString& title, const QString& text, const Qt::TextFormat format,
                      const std::optional<MessageAction>& action = std::nullopt) {
  InformationDialog dialog(parent);
  dialog.setWindowTitle(title);
  dialog.setHeaderStyle(DialogWindow::HeaderStyle::Message);
  auto* frame = new DialogLayout(&dialog, DialogContent::Scrollable, QSize(design::messageWidth, 0));
  auto* body = messageText(text, &dialog, format);
  auto font = body->font();
  font.setPixelSize(13);
  body->setFont(font);
  body->setLineHeight(13 * 1.9);
  body->setProperty("dialogTone", "muted");
  frame->body()->addWidget(body);
  if (action) {
    auto* buttons = new QDialogButtonBox(&dialog);
    auto* ok = buttons->addButton(action->text, QDialogButtonBox::AcceptRole);
    QObject::connect(ok, &QPushButton::clicked, &dialog, &QDialog::accept);
    setDialogAction(ok, action->style);
    ok->setAutoDefault(false);
    auto* cancel = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
    QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    cancel->setDefault(true);
    cancel->setFocus();
    frame->setActions(buttons);
  }
  return dialog.exec() == QDialog::Accepted;
}

}  // namespace

void warning(QWidget* parent, const QString& title, const QString& text, const Qt::TextFormat format) { (void)runMessageDialog(parent, title, text, format); }

void information(QWidget* parent, const QString& title, const QString& text, const Qt::TextFormat format) {
  (void)runMessageDialog(parent, title, text, format);
}

bool confirm(QWidget* parent, const QString& title, const QString& text, const Qt::TextFormat format) {
  return runMessageDialog(parent, title, text, format, MessageAction{I18n::tr("Continue"), DialogAction::Destructive});
}
bool confirm(QWidget* parent, const Confirmation& request) {
  return runMessageDialog(parent, request.title, request.text, request.format, MessageAction{request.acceptText, request.action});
}

}  // namespace uwf::ui::dialogs
