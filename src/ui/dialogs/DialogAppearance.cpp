// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DialogAppearance.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QSplitterHandle>
#include <QStyle>
#include <QToolButton>

#include "ui/appearance/ThemeManager.h"
#include "ui/appearance/WindowAppearance.h"

namespace uwf::ui {
namespace {

QString dialogStyle() { return ThemeManager::instance().styleSheet(QStringLiteral(":/styles/dialogs.qss")); }

class DialogWindowController final : public QObject {
 public:
  explicit DialogWindowController(QDialog* dialog) : QObject(dialog), m_dialog(dialog) {
    dialog->setProperty("uwfDialog", true);
    dialog->installEventFilter(this);
    qApp->installEventFilter(this);
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] { applyTheme(); });
  }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (event->type() == QEvent::ToolTip) {
      auto* widget = qobject_cast<QWidget*>(object);
      if (widget && widget->window() == m_dialog) {
        if (const auto* button = qobject_cast<QAbstractButton*>(widget)) {
          const auto* tool = qobject_cast<const QToolButton*>(button);
          const bool iconOnly = button->text().isEmpty() || (tool && tool->toolButtonStyle() == Qt::ToolButtonIconOnly);
          const bool explainsDisabled = !button->isEnabled() && button->toolTip() != button->text();
          if (!iconOnly && !explainsDisabled && !button->property("toolTipRequired").toBool()) return true;
        }
        if (const auto* label = qobject_cast<QLabel*>(widget); label && label->toolTip() == label->text()) {
          const bool fits = label->wordWrap() ? label->heightForWidth(label->width()) <= label->height()
                                              : label->fontMetrics().horizontalAdvance(label->text()) <= label->contentsRect().width();
          if (fits) return true;
        }
        if (const auto* view = qobject_cast<QAbstractItemView*>(widget->parentWidget()); view && view->viewport() == widget) {
          const auto index = view->indexAt(static_cast<QHelpEvent*>(event)->pos());
          const auto text = index.data(Qt::DisplayRole).toString();
          if (index.isValid() && index.data(Qt::ToolTipRole).toString() == text &&
              view->fontMetrics().horizontalAdvance(text) + 28 <= view->visualRect(index).width())
            return true;
        }
      }
    }
    if (object == m_dialog && (event->type() == QEvent::Show || event->type() == QEvent::WinIdChange)) applyWindowAppearance(m_dialog, Sem::Bg);
    if (event->type() == QEvent::KeyPress) {
      const auto* key = static_cast<QKeyEvent*>(event);
      const auto* widget = qobject_cast<QWidget*>(object);
      const auto* focus = m_dialog->focusWidget();
      const auto* combo = qobject_cast<const QComboBox*>(focus);
      if (object == m_dialog && (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) &&
          (qobject_cast<const QLineEdit*>(focus) || (combo && combo->isEditable()))) {
        // The editor has already handled Enter and emitted returnPressed if
        // its input is acceptable. Do not also activate a dialog default.
        event->accept();
        return true;
      }
      if ((key->key() == Qt::Key_Tab || key->key() == Qt::Key_Backtab) && widget && widget->window() == m_dialog) {
        // Qt rebuilds button-box focus proxies after style and visibility changes.
        // Traverse the buttons directly so entering a group cannot revisit its default button.
        for (const auto boxes = m_dialog->findChildren<QDialogButtonBox*>(); auto* box : boxes)
          if (box->window() == m_dialog) box->setFocusProxy(nullptr);
      }
    }
    if (event->type() == QEvent::MouseButtonPress) {
      auto* widget = qobject_cast<QWidget*>(object);
      auto* mouse = static_cast<QMouseEvent*>(event);
      if (mouse->button() == Qt::LeftButton && widget && widget->window() == m_dialog) {
        if (!isDragTarget(widget)) return false;
        auto* hit = m_dialog->childAt(m_dialog->mapFromGlobal(mouse->globalPosition().toPoint()));
        if (hit && !isDragTarget(hit)) return false;
        startWindowDrag(m_dialog, mouse->globalPosition().toPoint());
        return true;
      }
    }
    return QObject::eventFilter(object, event);
  }

 private:
  bool isDragTarget(QWidget* widget) const {
    for (auto* current = widget; current && current != m_dialog; current = current->parentWidget()) {
      if (current->property("dialogScrollHost").toBool()) continue;
      if (qobject_cast<QAbstractButton*>(current) || qobject_cast<QAbstractSlider*>(current) || qobject_cast<QLineEdit*>(current) ||
          qobject_cast<QAbstractSpinBox*>(current) || qobject_cast<QComboBox*>(current) || qobject_cast<QSplitterHandle*>(current))
        return false;
      if (qobject_cast<QAbstractScrollArea*>(current) && !current->property("dialogScrollHost").toBool()) return false;
      if (const auto* label = qobject_cast<QLabel*>(current); label && label->textInteractionFlags() != Qt::NoTextInteraction) return false;
      if (current->focusPolicy() != Qt::NoFocus) return false;
    }
    return true;
  }

  void applyTheme() {
    const auto style = dialogStyle();
    if (m_dialog->styleSheet() != style) m_dialog->setStyleSheet(style);
    applyWindowAppearance(m_dialog, Sem::Bg);
  }

  QDialog* m_dialog;
};

}  // namespace

void configureDialogWindow(QDialog* dialog) {
  if (!dialog->property("dialogKind").isValid()) dialog->setProperty("dialogKind", "panel");
  if (!dialog->property("uwfDialog").toBool()) new DialogWindowController(dialog);
}

void setDialogAction(QPushButton* button, const DialogAction action) {
  const char* role = action == DialogAction::Primary                ? "primary"
                     : action == DialogAction::Destructive          ? "destructive"
                     : action == DialogAction::DestructiveSecondary ? "destructiveSecondary"
                     : action == DialogAction::Close                ? "close"
                                                                    : "secondary";
  if (button->property("dialogAction").toString() == QLatin1String(role)) return;
  button->setProperty("dialogAction", role);
  button->style()->unpolish(button);
  button->style()->polish(button);
  button->update();
}


}  // namespace uwf::ui
