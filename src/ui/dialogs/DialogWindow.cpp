// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "DialogWindow.h"

#include <windows.h>

#include <QContextMenuEvent>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>

#include "DialogAppearance.h"
#include "DialogCloseButton.h"
#include "ui/appearance/WindowAppearance.h"
#include "ui/design/ControlShape.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/SurfaceShadow.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/Surface.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {
DialogWindow::DialogWindow(const Kind kind, QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      m_kind(kind),
      m_header(new QWidget(this)),
      m_eyebrow(new TextLabel(QStringLiteral("UWF Manager"), TextStyle::Eyebrow, m_header)),
      m_title(new TextLabel({}, TextStyle::Title, m_header)),
      m_description(new TextLabel({}, TextStyle::Subtitle, m_header)),
      m_headerIcon(new Surface(design::Color::SurfaceSoft, m_header)),
      m_headerGlyph(new IconView({}, 23, design::Color::Text, m_headerIcon)),
      m_close(nullptr) {
  setProperty("dialogKind", kind == Kind::Information ? "information" : "panel");
  setProperty("designDialogSurface", true);
  prepareWindowBackdrop(this);
  setContentsMargins(design::windowBorder, design::windowBorder, design::windowBorder, design::windowBorder);
  configureDialogWindow(this);
  installSurfaceShadow(this, ShadowKind::Window);
  m_close = new DialogCloseButton(m_header);
  bindRejectionButton(m_close);
  m_header->setObjectName(QStringLiteral("dialogHeader"));
  m_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  // Window controls stay on the physical right; the title follows the content direction.
  m_header->setLayoutDirection(Qt::LeftToRight);
  auto* stack = new QVBoxLayout(m_header);
  stack->setContentsMargins(design::dialogBodyPaddingX, design::dialogHeaderTop, design::dialogBodyPaddingX, design::dialogHeaderBottom);
  stack->setSpacing(8);
  m_eyebrow->setProperty("designTone", "faint");
  auto eyebrowFont = m_eyebrow->font();
  eyebrowFont.setLetterSpacing(QFont::AbsoluteSpacing, .65);
  m_eyebrow->setFont(eyebrowFont);
  stack->addWidget(m_eyebrow);
  auto* row = new QHBoxLayout();
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(14);
  m_headerIcon->setObjectName(QStringLiteral("dialogHeaderIcon"));
  m_headerIcon->setFixedSize(42, 42);
  static_cast<Surface*>(m_headerIcon)->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 7);
  auto* iconLayout = new QVBoxLayout(m_headerIcon);
  iconLayout->setContentsMargins(1, 1, 1, 1);
  iconLayout->addWidget(m_headerGlyph, 0, Qt::AlignCenter);
  m_headerIcon->hide();
  row->addWidget(m_headerIcon);
  m_title->setObjectName(QStringLiteral("dialogTitle"));
  m_title->setLayoutDirection(layoutDirection());
  m_title->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
  m_title->setWordWrap(true);
  updateTitleFont();
  row->addWidget(m_title, 1);
  stack->addLayout(row);
  m_description->setWordWrap(true);
  m_description->setProperty("designTone", "muted");
  m_description->hide();
  stack->addWidget(m_description);
  m_header->installEventFilter(this);
  m_close->raise();
  m_close->setDefault(kind == Kind::Information);
  connect(this, &QWidget::windowTitleChanged, static_cast<TextLabel*>(m_title), &TextLabel::setText);
  connect(m_close, &QPushButton::clicked, this, &QDialog::reject);
  auto* systemMenu = new QShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Space), this);
  connect(systemMenu, &QShortcut::activated, this, [this] { showSystemMenu(m_header->mapToGlobal(QPoint(0, m_header->height()))); });
}

void DialogWindow::bindRejectionButton(QAbstractButton* button) {
  button->setEnabled(m_rejectionEnabled);
  connect(this, &DialogWindow::rejectionEnabledChanged, button, &QWidget::setEnabled);
}

void DialogWindow::setRejectionEnabled(const bool enabled) {
  if (m_rejectionEnabled == enabled) return;
  m_rejectionEnabled = enabled;
  emit rejectionEnabledChanged(enabled);
}

bool DialogWindow::requestRejection() { return true; }

void DialogWindow::reject() { done(QDialog::Rejected); }

void DialogWindow::done(const int result) {
  if (result == QDialog::Rejected && (!m_rejectionEnabled || !requestRejection())) return;
  QDialog::done(result);
}

void DialogWindow::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  const auto& system = DesignSystem::instance();
  const QColor fill = property("systemBackdropActive").toBool() ? QColor(Qt::transparent) : system.color(design::Color::Window);
  if (fill.alpha() == 0) {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  }
  paintControlShape(painter, rect(), design::windowRadius, fill, system.color(design::Color::Line));
}

void DialogWindow::setDescription(const QString& text) {
  static_cast<TextLabel*>(m_description)->setText(text);
  m_description->setVisible(!text.isEmpty());
}

void DialogWindow::setHeaderIcon(const QIcon& icon) {
  m_headerGlyph->setIcon(icon);
  m_headerIcon->setVisible(!icon.isNull());
}
void DialogWindow::setHeaderStyle(const HeaderStyle style) {
  m_headerStyle = style;
  auto* stack = qobject_cast<QVBoxLayout*>(m_header->layout());
  const bool message = style == HeaderStyle::Message;
  stack->setContentsMargins(message ? design::messageBodyPaddingX : design::dialogBodyPaddingX,
                            message                       ? design::messageHeaderTop
                            : style == HeaderStyle::Power ? 30
                                                          : design::dialogHeaderTop,
                            message ? design::messageBodyPaddingX : design::dialogBodyPaddingX,
                            message                       ? design::messageHeaderBottom
                            : style == HeaderStyle::Power ? 20
                                                          : design::dialogHeaderBottom);
  m_eyebrow->setVisible(!message);
  m_description->setContentsMargins(0, style == HeaderStyle::Power ? 10 : 0, 0, 0);
  updateTitleFont();
}

bool DialogWindow::eventFilter(QObject* object, QEvent* event) {
  if (object == m_header && event->type() == QEvent::Resize) {
    m_close->move(m_header->width() - m_close->width() - 12, 12);
    m_close->raise();
  }
  return QDialog::eventFilter(object, event);
}

void DialogWindow::updateTitleFont() {
  const bool message = m_headerStyle == HeaderStyle::Message;
  static_cast<TextLabel*>(m_title)->setTextStyle(message ? TextStyle::MessageTitle : TextStyle::Title);
  m_title->setContentsMargins(0, 0, message ? design::messageTitlePaddingRight : 28, 0);
  auto titleFont = font();
  titleFont.setPixelSize(message ? design::messageTitleFontSize : m_headerStyle == HeaderStyle::Power ? 23 : design::titleFontSize);
  titleFont.setWeight(QFont::DemiBold);
  titleFont.setLetterSpacing(QFont::AbsoluteSpacing, message ? 0 : -.55);
  m_title->setFont(titleFont);
}

void DialogWindow::changeEvent(QEvent* event) {
  QDialog::changeEvent(event);
  if (event->type() == QEvent::LayoutDirectionChange) m_title->setLayoutDirection(layoutDirection());
  if (event->type() == QEvent::FontChange) updateTitleFont();
}

void DialogWindow::showSystemMenu(const QPoint& position) { showWindowMenu(this, position); }

void DialogWindow::contextMenuEvent(QContextMenuEvent* event) {
  const QPoint position = m_header->mapFromGlobal(event->globalPos());
  if (m_header->rect().contains(position) && !m_close->geometry().contains(position)) {
    showSystemMenu(event->globalPos());
    event->accept();
    return;
  }
  QDialog::contextMenuEvent(event);
}

void DialogWindow::showEvent(QShowEvent* event) {
  if (m_kind == Kind::Panel) {
    // Keep the initial focus in the workflow; the window close control is the last Tab stop.
    QWidget* first = nullptr;
    QWidget* last = nullptr;
    for (auto* widget = nextInFocusChain(); widget != this; widget = widget->nextInFocusChain())
      if (widget != m_close && widget->window() == this && widget->isVisibleTo(this) && widget->isEnabled() && widget->focusPolicy() & Qt::TabFocus) {
        if (!first) first = widget;
        last = widget;
      }
    if (last) QWidget::setTabOrder(last, m_close);
    if (first && (!focusWidget() || focusWidget() == m_close)) first->setFocus(Qt::OtherFocusReason);
  }
  QDialog::showEvent(event);
}

bool DialogWindow::nativeEvent(const QByteArray& type, void* message, qintptr* result) {
  return handleWindowResize(this, message, result) || handleWindowMenu(this, message, result) || QDialog::nativeEvent(type, message, result);
}
}  // namespace uwf::ui
#include "moc_DialogWindow.cpp"
