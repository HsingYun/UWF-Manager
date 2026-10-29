// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "SelectBox.h"

#include <QApplication>
#include <QCursor>
#include <QHideEvent>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QStyleOptionComboBox>
#include <QStylePainter>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <algorithm>

#include "ui/design/ControlShape.h"
#include "ui/design/DesignSystem.h"
#include "ui/design/FocusIndicator.h"
#include "ui/design/IconPainting.h"
#include "ui/design/ItemHighlight.h"
#include "ui/design/PopupAppearance.h"
#include "ui/design/PopupPlacement.h"
#include "ui/design/TextPainting.h"
namespace uwf::ui {
namespace {
class OptionDelegate final : public QStyledItemDelegate {
 public:
  explicit OptionDelegate(SelectBox* owner) : QStyledItemDelegate(owner), m_owner(owner) {}
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QFont font = option.font;
    font.setPixelSize(13);
    return {QFontMetrics(font).horizontalAdvance(index.data().toString()) + 70, design::menuRowHeight};
  }
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    using design::Color;
    auto& system = DesignSystem::instance();
    painter->save();
    const bool enabled = index.flags().testFlag(Qt::ItemIsEnabled);
    if (!enabled) painter->setOpacity(.4);
    const bool active =
        enabled && (option.state.testFlag(QStyle::State_MouseOver) || (FocusIndicator::keyboardNavigation() && option.state.testFlag(QStyle::State_Selected)));
    const qreal highlight = ItemHighlight::opacity(option.widget, option.rect.topLeft(), active);
    paintControlShape(*painter, option.rect, 4, DesignSystem::mix(Qt::transparent, system.color(Color::Hover), highlight));
    QFont font = option.font;
    font.setPixelSize(13);
    painter->setFont(font);
    painter->setPen(system.color(Color::Text));
    const bool rtl = option.direction == Qt::RightToLeft;
    const QRect label = option.rect.adjusted(rtl ? 35 : 10, 0, rtl ? -10 : -35, 0);
    paintTextLine(*painter, label, QFontMetrics(font).elidedText(index.data().toString(), Qt::ElideRight, label.width()), font);
    if (index.row() == m_owner->currentIndex()) {
      const qreal left = rtl ? option.rect.left() + 10 : option.rect.right() - 24;
      const qreal top = QRectF(option.rect).center().y() - 7.5;
      QPainterPath check;
      check.moveTo(left + 3.125, top + 7.5);
      check.lineTo(left + 5.625, top + 10);
      check.lineTo(left + 11.875, top + 3.75);
      paintIconPath(*painter, check, QRectF(left, top, 15, 15), system.color(Color::Muted), 1.1);
    }
    painter->restore();
  }

 private:
  SelectBox* m_owner;
};
}  // namespace
class SelectPopup final : public QWidget {
 public:
  explicit SelectPopup(SelectBox* owner) : QWidget(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint), m_owner(owner) {
    configurePopupAppearance(this);
    bindPopupToAnchor(this, owner);
    setObjectName(QStringLiteral("designSelectPopup"));
  }
  void detachOwner() { m_owner.clear(); }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    paintPopupSurface(painter, rect());
  }
  void hideEvent(QHideEvent* event) override {
    QWidget::hideEvent(event);
    if (m_owner) m_owner->hidePopup();
  }

 private:
  QPointer<SelectBox> m_owner;
};
SelectBox::SelectBox(QWidget* parent) : QComboBox(parent) {
  setCursor(Qt::PointingHandCursor);
  setSizeAdjustPolicy(QComboBox::AdjustToContents);
}
SelectBox::~SelectBox() {
  if (m_popup) {
    m_popup->detachOwner();
    m_popup->hide();
  }
}
void SelectBox::showPopup() {
  if (!isEnabled() || count() == 0) return;
  if (!m_popup) {
    m_popup = new SelectPopup(this);
    auto* layout = new QVBoxLayout(m_popup);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(0);
    m_options = new QListView(m_popup);
    m_options->setObjectName(QStringLiteral("selectOptions"));
    m_options->setFrameShape(QFrame::NoFrame);
    // The popup sizes its viewport from the visible option count. A list's
    // default minimum height would add an unselectable area below a single row.
    m_options->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    m_options->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_options->setSelectionMode(QAbstractItemView::SingleSelection);
    m_options->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_options->setMouseTracking(true);
    m_options->setUniformItemSizes(true);
    m_options->setItemDelegate(new OptionDelegate(this));
    m_popup->installEventFilter(this);
    m_options->installEventFilter(this);
    m_options->viewport()->installEventFilter(this);
    layout->addWidget(m_options);
    connect(m_options, &QListView::clicked, this, &SelectBox::acceptIndex);
    connect(m_options, &QListView::entered, m_options, &QListView::setCurrentIndex);
  }
  if (m_options->model() != model()) {
    m_options->setModel(model());
    connect(m_options->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& index) {
      if (index.isValid()) {
        emit highlighted(index.row());
        emit textHighlighted(index.data().toString());
      }
    });
  }
  m_options->setRootIndex(rootModelIndex());
  m_options->setModelColumn(modelColumn());
  {
    const QSignalBlocker blocker(m_options->selectionModel());
    m_options->setCurrentIndex(model()->index(currentIndex(), modelColumn(), rootModelIndex()));
  }
  const auto available = screen()->availableGeometry();
  const int rows = std::min(count(), maxVisibleItems());
  const int popupHeight = std::min(rows * design::menuRowHeight + 12, available.height());
  const int scrollbarWidth = count() > (popupHeight - 12) / design::menuRowHeight ? m_options->verticalScrollBar()->sizeHint().width() : 0;
  int popupWidth = std::max(120, width());
  for (int row = 0; row < count(); ++row)
    popupWidth = std::max(popupWidth, m_options->sizeHintForIndex(model()->index(row, modelColumn(), rootModelIndex())).width() + 12 + scrollbarWidth);
  popupWidth = std::min(popupWidth, available.width());
  m_popup->setGeometry(anchoredPopupGeometry(QRect(mapToGlobal(QPoint()), size()), QSize(popupWidth, popupHeight), available, layoutDirection()));
  setProperty("designExpanded", true);
  m_popup->show();
  m_options->setFocus(Qt::PopupFocusReason);
  m_options->scrollTo(m_options->currentIndex());
  update();
}
void SelectBox::hidePopup() {
  if (m_popup && m_popup->isVisible()) m_popup->hide();
  QComboBox::hidePopup();
  m_pointerDown = false;
  m_hovered = rect().contains(mapFromGlobal(QCursor::pos()));
  setProperty("designExpanded", false);
  update();
}
void SelectBox::acceptIndex(const QModelIndex& index) {
  if (!isEnabled()) {
    hidePopup();
    return;
  }
  if (!index.isValid() || !index.flags().testFlag(Qt::ItemIsEnabled) || !index.flags().testFlag(Qt::ItemIsSelectable)) return;
  const int row = index.row();
  const QString text = index.data().toString();
  setCurrentIndex(row);
  hidePopup();
  emit activated(row);
  emit textActivated(text);
}
void SelectBox::paintEvent(QPaintEvent*) {
  QStylePainter painter(this);
  QStyleOptionComboBox option;
  initStyleOption(&option);
  option.state.setFlag(QStyle::State_Sunken, m_pointerDown);
  option.state.setFlag(QStyle::State_MouseOver, m_hovered);
  painter.drawComplexControl(QStyle::CC_ComboBox, option);
  painter.drawControl(QStyle::CE_ComboBoxLabel, option);
}
void SelectBox::mousePressEvent(QMouseEvent* event) {
  if (isEditable() || event->button() != Qt::LeftButton) {
    QComboBox::mousePressEvent(event);
    return;
  }
  m_pointerDown = true;
  m_hovered = true;
  showPopup();
  event->accept();
  update();
}
void SelectBox::mouseReleaseEvent(QMouseEvent* event) {
  m_pointerDown = false;
  if (isEditable())
    QComboBox::mouseReleaseEvent(event);
  else
    event->accept();
  update();
}
void SelectBox::enterEvent(QEnterEvent* event) {
  m_hovered = true;
  QComboBox::enterEvent(event);
  update();
}
void SelectBox::leaveEvent(QEvent* event) {
  m_hovered = false;
  QComboBox::leaveEvent(event);
  update();
}
void SelectBox::changeEvent(QEvent* event) {
  QComboBox::changeEvent(event);
  if (event->type() == QEvent::EnabledChange) {
    if (!isEnabled()) hidePopup();
    m_hovered = isEnabled() && rect().contains(mapFromGlobal(QCursor::pos()));
    update();
  }
}
bool SelectBox::eventFilter(QObject* object, QEvent* event) {
  if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
    const auto* mouse = static_cast<QMouseEvent*>(event);
    if (event->type() == QEvent::MouseButtonRelease) m_pointerDown = false;
    m_hovered = rect().contains(mapFromGlobal(mouse->globalPosition().toPoint()));
    update();
  }
  if (object == m_options && event->type() == QEvent::KeyPress) {
    auto* key = static_cast<QKeyEvent*>(event);
    if (key->key() == Qt::Key_Escape || (key->key() == Qt::Key_Up && key->modifiers().testFlag(Qt::AltModifier))) {
      hidePopup();
      return true;
    }
    if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter || key->key() == Qt::Key_Space) {
      acceptIndex(m_options->currentIndex());
      return true;
    }
    if (key->key() == Qt::Key_Tab || key->key() == Qt::Key_Backtab) {
      hidePopup();
      focusNextPrevChild(key->key() != Qt::Key_Backtab && !key->modifiers().testFlag(Qt::ShiftModifier));
      return true;
    }
  }
  return QComboBox::eventFilter(object, event);
}
}  // namespace uwf::ui
#include "moc_SelectBox.cpp"
