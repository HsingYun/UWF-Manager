// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WindowTitleBar.h"

#include <windows.h>

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>

#include "ui/appearance/I18n.h"
#include "ui/widgets/CommandMenu.h"
#include "ui/widgets/TextLabel.h"
namespace uwf::ui {
namespace {
QPixmap centeredApplicationIcon(const QSize& size, const qreal scale) {
  const auto icon = QIcon(QStringLiteral(":/icons/app.svg")).pixmap(size, scale);
  const auto pixels = icon.toImage();
  QRect ink;
  for (int y = 0; y < pixels.height(); ++y)
    for (int x = 0; x < pixels.width(); ++x)
      if (qAlpha(pixels.pixel(x, y)) > 0) ink = ink.united(QRect(x, y, 1, 1));
  QPixmap centered(icon.size());
  centered.setDevicePixelRatio(icon.devicePixelRatio());
  centered.fill(Qt::transparent);
  QPainter painter(&centered);
  const QPointF offset = (QRectF(pixels.rect()).center() - QRectF(ink).center()) / icon.devicePixelRatio();
  painter.drawPixmap(offset, icon);
  return centered;
}
enum class WindowCommand { Minimize, Maximize, Close };
class WindowButton final : public QPushButton {
 public:
  WindowButton(const WindowCommand command, QWidget* parent) : QPushButton(parent), m_command(command) {
    setFixedSize(43, 49);
    setFlat(true);
    setProperty("designRole", "quiet");
    setProperty("designRadius", 0);
    setProperty("nativeCaptionHit", command == WindowCommand::Minimize ? HTMINBUTTON : command == WindowCommand::Maximize ? HTMAXBUTTON : HTCLOSE);
    connect(this, &QPushButton::clicked, this, [this] {
      auto* target = window();
      switch (m_command) {
        case WindowCommand::Minimize:
          SendMessageW(reinterpret_cast<HWND>(target->winId()), WM_SYSCOMMAND, SC_MINIMIZE, 0);
          break;
        case WindowCommand::Maximize:
          SendMessageW(reinterpret_cast<HWND>(target->winId()), WM_SYSCOMMAND, target->isMaximized() ? SC_RESTORE : SC_MAXIMIZE, 0);
          break;
        case WindowCommand::Close:
          SendMessageW(reinterpret_cast<HWND>(target->winId()), WM_SYSCOMMAND, SC_CLOSE, 0);
          break;
      }
    });
    window()->installEventFilter(this);
    updateCommandName();
    I18n::bind(this, [this] { updateCommandName(); });
  }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override {
    if (object == window() && event->type() == QEvent::WindowStateChange) updateCommandName();
    return QPushButton::eventFilter(object, event);
  }

  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    auto& system = DesignSystem::instance();
    const bool hover = (underMouse() || property("nativeCaptionHover").toBool()) && isEnabled();
    if (hover || isDown())
      p.fillRect(rect(), system.color(m_command == WindowCommand::Close ? (isDown() ? design::Color::ClosePressed : design::Color::CloseHover)
                                                                        : (isDown() ? design::Color::ButtonPressed : design::Color::ButtonHover)));
    p.setPen(QPen(hover && m_command == WindowCommand::Close ? Qt::white : system.color(design::Color::Text), 1.0));
    const QPointF center = QRectF(rect()).center();
    if (m_command == WindowCommand::Minimize)
      p.drawLine(center + QPointF(-5, 0), center + QPointF(5, 0));
    else if (m_command == WindowCommand::Close) {
      p.drawLine(center + QPointF(-4, -4), center + QPointF(4, 4));
      p.drawLine(center + QPointF(4, -4), center + QPointF(-4, 4));
    } else if (window()->isMaximized()) {
      // Draw only the exposed back edges; an opaque erase rectangle would
      // leave a solid patch in the system-material title bar.
      p.drawLine(center + QPointF(-3, -5), center + QPointF(5, -5));
      p.drawLine(center + QPointF(5, -5), center + QPointF(5, 3));
      p.drawRect(QRectF(center + QPointF(-5, -3), QSizeF(8, 8)));
    } else
      p.drawRect(QRectF(center + QPointF(-4.5, -4.5), QSizeF(9, 9)));
  }

 private:
  void updateCommandName() {
    const QString name = m_command == WindowCommand::Minimize   ? I18n::tr("Minimize")
                         : m_command == WindowCommand::Maximize ? (window()->isMaximized() ? I18n::tr("Restore") : I18n::tr("Maximize"))
                                                                : I18n::tr("Close");
    setProperty("accessibleName", name);
    setToolTip(name);
  }
  WindowCommand m_command;
};
}  // namespace
WindowTitleBar::WindowTitleBar(QWidget* parent) : Surface(design::Color::Window, parent), m_utilities(new QHBoxLayout()) {
  setObjectName(QStringLiteral("mainTitleBar"));
  setProperty("windowDragSurface", true);
  setFixedHeight(49);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(21, 0, 0, 0);
  row->setSpacing(0);
  auto* appIcon = new QLabel(this);
  appIcon->setObjectName(QStringLiteral("mainApplicationIcon"));
  appIcon->setAlignment(Qt::AlignCenter);
  appIcon->setPixmap(centeredApplicationIcon(QSize(25, 25), devicePixelRatioF()));
  appIcon->setFixedSize(25, 25);
  row->addWidget(appIcon, 0, Qt::AlignVCenter);
  row->addSpacing(10);
  auto* title = new TextLabel(I18n::applicationTitle(), TextStyle::Label, this);
  title->setObjectName(QStringLiteral("mainApplicationName"));
  title->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
  title->setElideMode(Qt::ElideRight);
  title->setFixedHeight(25);
  auto titleFont = title->font();
  titleFont.setWeight(QFont::DemiBold);
  title->setFont(titleFont);
  row->addWidget(title, 0, Qt::AlignVCenter);
  I18n::bind(this, [this, row, title] {
    title->setText(I18n::applicationTitle());
    row->setContentsMargins(layoutDirection() == Qt::RightToLeft ? 0 : 21, 0, layoutDirection() == Qt::RightToLeft ? 21 : 0, 0);
  });
  row->setContentsMargins(layoutDirection() == Qt::RightToLeft ? 0 : 21, 0, layoutDirection() == Qt::RightToLeft ? 21 : 0, 0);
  row->addStretch();
  m_utilities->setSpacing(4);
  row->addLayout(m_utilities);
  row->addSpacing(16);
  for (auto command : {WindowCommand::Minimize, WindowCommand::Maximize, WindowCommand::Close}) {
    auto* button = new WindowButton(command, this);
    m_windowCommands.append(button);
    row->addWidget(button);
  }
}
void WindowTitleBar::addUtility(QAction* action) {
  auto* button = new QToolButton(this);
  button->setAutoRaise(true);
  button->setProperty("designRole", "quiet");
  button->setDefaultAction(action);
  button->setFixedSize(30, 30);
  button->setIconSize({16, 16});
  button->setToolButtonStyle(Qt::ToolButtonIconOnly);
  if (action->isCheckable()) {
    button->setProperty("designCheckedIconOnly", true);
    button->setObjectName(QStringLiteral("hubVisibilityButton"));
  }
  if (action->menu()) {
    button->setPopupMode(QToolButton::InstantPopup);
    if (auto* menu = qobject_cast<CommandMenu*>(action->menu())) menu->setAnchor(button);
  }
  m_utilities->addWidget(button);
  m_focusOrder.append(button);
  const auto order = m_focusOrder + m_windowCommands;
  for (int index = 1; index < order.size(); ++index) QWidget::setTabOrder(order[index - 1], order[index]);
}
}  // namespace uwf::ui
