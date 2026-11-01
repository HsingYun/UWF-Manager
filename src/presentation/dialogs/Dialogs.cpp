// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/dialogs/Dialogs.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QPushButton>
#include <optional>

#include "design/DesignTokens.h"
#include "presentation/appearance/I18n.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogLayout.h"
#include "presentation/dialogs/InformationDialog.h"

namespace uwf::ui::dialogs {

namespace {

ContentHintBox* messageText(const QString& text, QWidget* parent, const Qt::TextFormat format = Qt::PlainText) {
  auto* body = new ContentHintBox(parent);
  if (format == Qt::RichText)
    body->setHtml(text);
  else
    body->setPlainText(text);
  body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  return body;
}

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
