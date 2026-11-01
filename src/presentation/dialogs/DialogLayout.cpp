// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/dialogs/DialogLayout.h"

#include <QAbstractScrollArea>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QScopedValueRollback>
#include <QScreen>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>
#include <algorithm>
#include <utility>

#include "design/DesignTokens.h"
#include "presentation/controls/layout/FlexibleColumnLayout.h"
#include "presentation/controls/layout/VerticalScrollArea.h"
#include "presentation/controls/pagination/PagedWindowLayout.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogAppearance.h"
#include "presentation/dialogs/DialogWindow.h"

namespace uwf::ui {
namespace {
class DialogFooter final : public QFrame {
 public:
  explicit DialogFooter(QWidget* parent) : QFrame(parent) { connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update)); }
  void setDividerColor(design::Color color) {
    m_divider = color;
    update();
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    const auto& system = DesignSystem::instance();
    auto fill = system.color(design::Color::Footer);
    if (window()->property("systemBackdropActive").toBool()) fill.setAlphaF(.5);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath shape;
    shape.addRoundedRect(QRectF(rect()).adjusted(0, -8, 0, 0), 8, 8);
    painter.fillPath(shape, fill);
    painter.setPen(system.color(m_divider));
    painter.drawLine(0, 0, width() - 1, 0);
  }

 private:
  design::Color m_divider = design::Color::Line;
};
}  // namespace

DialogLayout::DialogLayout(QDialog* dialog, const DialogContent content, const QSize initialSize, const QSize minimumSize)
    : QVBoxLayout(dialog),
      m_dialog(dialog),
      m_content(new QWidget(dialog)),
      m_body(new FlexibleColumnLayout(m_content)),
      m_footer(new DialogFooter(dialog)),
      m_fitContent(initialSize.height() <= 0),
      m_initialHeight(std::max(0, initialSize.height())),
      m_actionPaddingX(design::footerPaddingX),
      m_actionPaddingY(design::footerPaddingY),
      m_actionHeight(design::footerHeight),
      m_minimumSize(minimumSize.expandedTo(QSize(0, 0))) {
  configureDialogWindow(dialog);
  setContentsMargins(0, 0, 0, 0);
  setSpacing(0);
  setSizeConstraint(QLayout::SetNoConstraint);
  // A scroll viewport clips outlines outside its contents. Keep the first
  // control's keyboard focus frame inside the body, as the side padding does.
  const int bodyTopPadding = content == DialogContent::Scrollable ? dialog->style()->pixelMetric(QStyle::PM_FocusFrameVMargin) : 0;
  m_body->setContentsMargins(design::dialogBodyPaddingX, bodyTopPadding, design::dialogBodyPaddingX, design::dialogBodyPaddingBottom);
  m_body->setSpacing(design::dialogBodyGap);
  m_footer->setObjectName(QStringLiteral("dialogFooter"));
  m_footer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_footer->hide();
  if (auto* window = qobject_cast<DialogWindow*>(dialog)) {
    const bool message = window->m_headerStyle == DialogWindow::HeaderStyle::Message;
    m_actionPaddingX = message ? design::messageFooterPaddingX : window->m_headerStyle == DialogWindow::HeaderStyle::Power ? 27 : design::footerPaddingX;
    m_actionPaddingY = message ? design::messageFooterPaddingY : design::footerPaddingY;
    m_actionHeight = message ? design::messageFooterHeight : design::footerHeight;
    if (message) m_body->setContentsMargins(design::messageBodyPaddingX, bodyTopPadding, design::messageBodyPaddingX, design::messageBodyPaddingBottom);
    m_header = window->m_header;
    addWidget(m_header);
  }
  if (content == DialogContent::Scrollable) {
    m_scroll = new VerticalScrollArea(dialog);
    m_scroll->setProperty("dialogScrollHost", true);
    m_scroll->setFocusPolicy(Qt::NoFocus);
    m_scroll->setMinimumHeight(48);
    m_scroll->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    connect(m_scroll->verticalScrollBar(), &QScrollBar::rangeChanged, this,
            [this](int minimum, int maximum) { m_scroll->setFocusPolicy(maximum > minimum ? Qt::StrongFocus : Qt::NoFocus); });
    m_scroll->setWidget(m_content);
    m_content->setAutoFillBackground(false);
    addWidget(m_scroll, 1);
  } else {
    addWidget(m_content, 1);
  }
  addWidget(m_footer);
  dialog->setMinimumSize(minimumSize.expandedTo(QSize(0, 0)));
  dialog->resize(initialSize.width() > 0 ? initialSize.width() : 520, initialSize.height() > 0 ? initialSize.height() : 320);
  dialog->installEventFilter(this);
  m_content->installEventFilter(this);
  m_footer->installEventFilter(this);
}

void DialogLayout::setActions(QDialogButtonBox* buttons) { setActionWidgets(buttons, nullptr); }
void DialogLayout::setActionMetrics(const int paddingX, const int paddingY, const int minimumHeight) {
  m_actionPaddingX = paddingX;
  m_actionPaddingY = paddingY;
  m_actionHeight = minimumHeight;
  if (auto* layout = m_footer->layout()) {
    layout->setContentsMargins(paddingX, paddingY, paddingX, paddingY);
    m_footer->updateGeometry();
  }
}
void DialogLayout::setDividerColor(const design::Color color) { static_cast<DialogFooter*>(m_footer)->setDividerColor(color); }

void DialogLayout::setActions(QDialogButtonBox* buttons, QLabel* hint) { setActionWidgets(buttons, hint); }
void DialogLayout::setActions(QDialogButtonBox* buttons, QWidget* leadingContent) { setActionWidgets(buttons, leadingContent); }
void DialogLayout::setActions(QWidget* actions, QWidget* leadingContent) { setActionWidgets(actions, leadingContent); }
void DialogLayout::setActions(QDialogButtonBox* buttons, QDialogButtonBox* auxiliary) { setActionWidgets(buttons, auxiliary); }

int DialogLayout::contentHeight() const { return m_body->hasHeightForWidth() ? m_body->totalHeightForWidth(m_content->width()) : m_body->sizeHint().height(); }

int DialogLayout::chromeHeight() const {
  const int header = m_header ? (m_header->hasHeightForWidth() ? m_header->heightForWidth(m_dialog->width()) : m_header->sizeHint().height()) : 0;
  return header + (m_footer->isHidden() ? 0 : m_footer->height());
}

void DialogLayout::setActions(QDialogButtonBox* buttons, QDialogButtonBox* auxiliary, QWidget* trailingInformation) {
  setActionWidgets(buttons, auxiliary, trailingInformation);
}

void DialogLayout::setActionWidgets(QWidget* buttons, QWidget* leading, QWidget* trailingInformation) {
  m_leadingContent = leading;
  if (leading) leading->installEventFilter(this);
  if (auto* window = qobject_cast<DialogWindow*>(m_dialog)) {
    const auto bindRejectionButtons = [window](QDialogButtonBox* box) {
      for (auto buttons = box->buttons(); auto* button : std::as_const(buttons))
        if (box->buttonRole(button) == QDialogButtonBox::RejectRole) window->bindRejectionButton(button);
    };
    if (auto* box = qobject_cast<QDialogButtonBox*>(buttons)) bindRejectionButtons(box);
    if (auto* auxiliary = qobject_cast<QDialogButtonBox*>(leading)) bindRejectionButtons(auxiliary);
  }
  const int height = std::max(m_actionHeight, m_dialog->fontMetrics().lineSpacing() * 2 + 24);
  auto* row = new QHBoxLayout(m_footer);
  row->setContentsMargins(m_actionPaddingX, m_actionPaddingY, m_actionPaddingX, m_actionPaddingY);
  row->setSpacing(16);
  if (auto* hint = qobject_cast<QLabel*>(leading)) {
    hint->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    hint->setWordWrap(true);
    hint->setProperty("dialogTone", "muted");
    int low = 1;
    int high = hint->fontMetrics().horizontalAdvance(hint->text());
    while (low < high) {
      const int width = low + (high - low) / 2;
      if (hint->heightForWidth(width) > height - m_actionPaddingY * 2)
        low = width + 1;
      else
        high = width;
    }
    hint->setMinimumWidth(low);
    row->addWidget(hint, 1, Qt::AlignVCenter);
  } else {
    const bool expanding = leading && !qobject_cast<QDialogButtonBox*>(leading) && leading->sizePolicy().horizontalPolicy() == QSizePolicy::Expanding;
    if (leading) row->addWidget(leading, expanding ? 1 : 0, Qt::AlignVCenter);
    if (!expanding) row->addStretch(1);
  }
  if (buttons) row->addWidget(buttons, 0, Qt::AlignVCenter);
  m_footer->setFixedHeight(height);
  if (trailingInformation) row->addWidget(trailingInformation, 0, Qt::AlignVCenter);
  m_footer->show();
}

bool DialogLayout::eventFilter(QObject* object, QEvent* event) {
  if (object == m_leadingContent && event->type() == QEvent::LayoutRequest && m_footer->layout()) m_footer->layout()->invalidate();
  // Fit a newly shown page before the scroll area processes its content layout
  // request. Deferring this step lets the page paint at the previous page's size.
  if (object == m_content && event->type() == QEvent::LayoutRequest && m_initialized && m_autoSizeToContent && m_fitContent && !m_fitting &&
      m_dialog->isVisible()) {
    fitToContent();
    return QVBoxLayout::eventFilter(object, event);
  }
  if (object == m_dialog && (event->type() == QEvent::Move || event->type() == QEvent::Resize) && m_initialized && !m_fitting) {
    m_center = m_dialog->frameGeometry().center();
  }
  if (object == m_dialog && event->type() == QEvent::Show && !m_initialized) {
    m_initialized = true;
    m_center = availableWorkArea().center();
    if (const auto* owner = m_dialog->parentWidget(); owner && owner->window()->isVisible()) m_center = owner->window()->frameGeometry().center();
    fitToContent();
  } else if (m_initialized && !m_fitting && !m_updateQueued &&
             (event->type() == QEvent::LayoutRequest || event->type() == QEvent::FontChange || event->type() == QEvent::ScreenChangeInternal)) {
    m_updateQueued = true;
    QTimer::singleShot(0, this, [this] {
      m_updateQueued = false;
      if (m_dialog->isVisible()) {
        if (m_autoSizeToContent && m_fitContent)
          fitToContent();
        else
          updateAdaptiveSize();
      }
    });
  }
  return QVBoxLayout::eventFilter(object, event);
}

void DialogLayout::fitToContent() {
  const QScopedValueRollback<bool> fitting(m_fitting, true);
  const auto available = availableClientSize();
  updateAdaptiveSize();
  activate();
  m_body->activate();
  // Fit the initial content, and refit when the caller opts into automatic
  // sizing. Other dialogs retain their outer geometry as content changes.
  if (m_fitContent) {
    const auto margins = m_dialog->contentsMargins();
    const int natural = contentHeight() + chromeHeight() + margins.top() + margins.bottom();
    m_dialog->resize(m_dialog->width(), std::min(natural, available.height()));
  } else {
    m_dialog->resize(m_dialog->width(), std::min(m_initialHeight, available.height()));
  }
  updateAdaptiveSize();
  m_dialog->resize(m_dialog->size().boundedTo(available));
  PagedWindowLayout::forWindow(m_dialog).fit();
  const auto workArea = availableWorkArea();
  const auto frame = m_dialog->frameGeometry();
  const QPoint position = m_center - QPoint(frame.width() / 2, frame.height() / 2);
  m_dialog->move(std::clamp(position.x(), workArea.left(), std::max(workArea.left(), workArea.right() - frame.width() + 1)),
                 std::clamp(position.y(), workArea.top(), std::max(workArea.top(), workArea.bottom() - frame.height() + 1)));
}

void DialogLayout::updateAdaptiveSize() {
  const QScopedValueRollback<bool> fitting(m_fitting, true);
  updateChromeMinimums();
  const auto margins = m_dialog->contentsMargins();
  const int chromeMinimum = chromeHeight() + margins.top() + margins.bottom() + (m_scroll ? 48 : contentHeight());
  const int headerWidth = m_header ? m_header->minimumSizeHint().width() : 0;
  const int footerWidth = m_footer->isHidden() ? 0 : m_footer->minimumSizeHint().width();
  const int chromeWidth = std::max(headerWidth, footerWidth) + margins.left() + margins.right();
  m_dialog->setMinimumSize(m_minimumSize.expandedTo(QSize(chromeWidth, chromeMinimum)).boundedTo(availableClientSize()));
}

void DialogLayout::updateChromeMinimums() {
  bool changed = false;
  if (!m_footer->isHidden() && m_footer->layout()) {
    const auto* layout = m_footer->layout();
    const int natural = layout->hasHeightForWidth() ? layout->totalHeightForWidth(m_dialog->contentsRect().width()) : layout->sizeHint().height();
    const int height = std::max({m_actionHeight, m_dialog->fontMetrics().lineSpacing() * 2 + 24, natural});
    if (m_footer->height() != height) {
      m_footer->setFixedHeight(height);
      changed = true;
    }
  }
  if (m_header) {
    const int height = m_header->hasHeightForWidth() ? m_header->heightForWidth(m_dialog->contentsRect().width()) : m_header->sizeHint().height();
    if (m_header->minimumHeight() != height) {
      m_header->setMinimumHeight(height);
      changed = true;
    }
  }
  for (auto boxes = m_footer->findChildren<QDialogButtonBox*>(); auto* box : std::as_const(boxes)) {
    box->ensurePolished();
    const int width = box->sizeHint().width();
    if (box->minimumWidth() == width) continue;
    box->setMinimumWidth(width);
    changed = true;
  }
  if (changed) {
    if (m_footer->layout()) m_footer->layout()->invalidate();
    invalidate();
    activate();
  }
}

QRect DialogLayout::availableWorkArea() const { return m_dialog->screen()->availableGeometry(); }

QSize DialogLayout::availableClientSize() const {
  const QSize frame = (m_dialog->frameGeometry().size() - m_dialog->size()).expandedTo(QSize(0, 0));
  return (availableWorkArea().size() - frame).expandedTo(QSize(1, 1));
}

}  // namespace uwf::ui
