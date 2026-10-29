// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ApplyPlanList.h"

#include <QHBoxLayout>
#include <QFontMetricsF>
#include <QPainter>
#include <QTextLayout>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

#include "presentation/configuration/ApplyPlanViewModel.h"
#include "ui/appearance/ThemeManager.h"
#include "ui/design/BadgePainting.h"
#include "ui/design/ControlShape.h"
#include "ui/design/DesignSystem.h"
#include "ui/widgets/ContentHintBox.h"
#include "ui/widgets/IconView.h"
#include "ui/widgets/RecyclingListView.h"
#include "ui/widgets/StatusBadge.h"
#include "ui/widgets/TextLabel.h"

namespace uwf::ui {
namespace {
using Model = presentation::ApplyPlanViewModel;
QFont rowFont(const int pixels, const QFont::Weight weight = QFont::Normal, const bool path = false) {
  auto font = DesignSystem::defaultFont();
  font.setPixelSize(pixels);
  font.setWeight(weight);
  if (path) font.setFamilies(DesignSystem::monospaceFamilies());
  return font;
}
int textHeight(QString text, const QFont& font, const int width) {
  text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  text.replace(u'\r', u'\n');
  text.replace(u'\n', QChar(QChar::LineSeparator));
  if (!text.contains(QChar(QChar::LineSeparator)) && !text.contains(u'\t') &&
      QFontMetricsF(font).horizontalAdvance(text) <= std::max(1, width))
    return static_cast<int>(std::ceil(font.pixelSize() * 1.6));
  QTextLayout layout(text, font);
  QTextOption option;
  option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  layout.setTextOption(option);
  layout.beginLayout();
  int lines = 0;
  while (true) {
    auto line = layout.createLine();
    if (!line.isValid()) break;
    line.setLineWidth(std::max(1, width));
    ++lines;
  }
  layout.endLayout();
  return static_cast<int>(std::ceil(lines * font.pixelSize() * 1.6));
}
class ChangeRow final : public QWidget {
 public:
  ChangeRow(const bool heading, QWidget* parent) : QWidget(parent), m_heading(heading) {
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(heading ? 8 : 16);
    if (heading) {
      m_icon = new IconView({}, 16, design::Color::Muted, this);
      m_title = new TextLabel({}, TextStyle::Caption, this);
      m_title->setFont(rowFont(12, QFont::DemiBold));
      layout->addWidget(m_icon);
      layout->addWidget(m_title, 1);
    } else {
      auto* name = new QVBoxLayout;
      name->setContentsMargins(0, 0, 0, 0);
      name->setSpacing(2);
      m_value = new ContentHintBox(this);
      m_value->setProperty("designPlainDocument", true);
      m_value->setLineHeight(20.8);
      m_value->setTextInteractionFlags(Qt::TextSelectableByMouse);
      m_value->setFocusPolicy(Qt::ClickFocus);
      name->addWidget(m_value);
      m_description = new TextLabel({}, TextStyle::Eyebrow, this);
      m_description->setColorRole(design::Color::Faint);
      m_description->setWordWrap(true);
      name->addWidget(m_description);
      layout->addLayout(name, 1);
      m_before = new TextLabel({}, TextStyle::Caption, this);
      m_before->setColorRole(design::Color::Faint);
      m_before->setWordWrap(true);
      m_before->setFixedWidth(130);
      m_arrow = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/arrow.svg")), 20, design::Color::Faint, this);
      auto* arrowCell = new QWidget(this);
      arrowCell->setFixedWidth(24);
      auto* arrowLayout = new QHBoxLayout(arrowCell);
      arrowLayout->setContentsMargins(0, 0, 0, 0);
      arrowLayout->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
      arrowLayout->addWidget(m_arrow);
      m_after = new TextLabel({}, TextStyle::Label, this);
      m_after->setFont(rowFont(13, QFont::Medium));
      m_after->setWordWrap(true);
      m_after->setFixedWidth(155);
      m_badge = new StatusBadge(this);
      layout->addWidget(m_before);
      layout->addWidget(arrowCell);
      layout->addWidget(m_after);
      layout->addWidget(m_badge);
    }
    connect(&DesignSystem::instance(), &DesignSystem::changed, this, qOverload<>(&QWidget::update));
  }
  void bind(const QModelIndex& index) {
    if (m_heading) {
      m_title->setText(index.data().toString());
      layout()->setContentsMargins(17, 11, 17, 10);
      m_icon->setIcon(ThemeManager::instance().icon(index.data(Qt::DecorationRole).toString()));
    } else {
      const bool path = index.data(Model::PathRole).toBool();
      m_last = index.data(Model::LastRole).toBool();
      m_value->setFont(rowFont(13, QFont::Normal, path));
      const auto title = index.data().toString();
      if (m_value->toPlainText() != title) m_value->setPlainText(title);
      m_description->setText(index.data(Model::DescriptionRole).toString());
      m_description->setVisible(!m_description->text().isEmpty());
      m_before->setText(index.data(Model::BeforeRole).toString());
      m_after->setText(index.data(Model::AfterRole).toString());
      m_badge->setText(m_after->text());
      m_before->setVisible(!path);
      m_arrow->parentWidget()->setVisible(!path);
      m_arrow->setVisible(!path);
      m_after->setVisible(!path);
      m_badge->setVisible(path);
      layout()->setContentsMargins(19, 11, 19, 11 + (m_last ? 17 : 0));
    }
    update();
  }
 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF bounds(rect());
    if (m_last) bounds.setBottom(bounds.bottom() - 16);
    painter.setClipRect(bounds);
    auto frame = bounds;
    if (m_heading) frame.setBottom(frame.bottom() + 6);
    else {
      frame.setTop(frame.top() - 6);
      if (!m_last) frame.setBottom(frame.bottom() + 6);
    }
    const auto& colors = DesignSystem::instance();
    paintControlShape(painter, frame, 6, colors.color(m_heading ? design::Color::SurfaceSoft : design::Color::Surface), colors.color(design::Color::Line));
    if (!m_last) {
      painter.setPen(colors.color(design::Color::Line));
      painter.drawLine(QPointF(0, bounds.bottom() - .5), QPointF(bounds.right(), bounds.bottom() - .5));
    }
  }
 private:
  bool m_heading;
  bool m_last = false;
  TextLabel* m_title = nullptr;
  ContentHintBox* m_value = nullptr;
  TextLabel* m_description = nullptr;
  TextLabel* m_before = nullptr;
  TextLabel* m_after = nullptr;
  IconView* m_icon = nullptr;
  IconView* m_arrow = nullptr;
  StatusBadge* m_badge = nullptr;
};
}
RecyclingListView* createApplyPlanList(Model& model, QWidget* commands, QWidget* parent) {
  auto factory = [commands](const int kind, QWidget* viewport) -> QWidget* {
    if (kind == Model::Commands) { commands->setParent(viewport); return commands; }
    return new ChangeRow(kind == Model::Heading, viewport);
  };
  auto bind = [](QWidget* widget, const QModelIndex& index) {
    if (index.data(Model::KindRole).toInt() != Model::Commands) static_cast<ChangeRow*>(widget)->bind(index);
  };
  auto measure = [commands](const QModelIndex& index, const int width) {
    const int kind = index.data(Model::KindRole).toInt();
    if (kind == Model::Heading) return QSize(width, 41);
    if (kind == Model::Commands) {
      auto* layout = commands->layout();
      return QSize(width, layout->hasHeightForWidth() ? layout->totalHeightForWidth(width) : layout->sizeHint().height());
    }
    const bool path = index.data(Model::PathRole).toBool();
    const int trailing = path ? statusBadgeSize(index.data(Model::AfterRole).toString(), rowFont(11), {}).width() + 16 : 130 + 24 + 155 + 48;
    const int nameWidth = std::max(1, width - 38 - trailing);
    int height = textHeight(index.data().toString(), rowFont(13, QFont::Normal, path), nameWidth);
    const auto description = index.data(Model::DescriptionRole).toString();
    if (!description.isEmpty()) height += 2 + textHeight(description, rowFont(11), nameWidth);
    if (!path) height = std::max({height, textHeight(index.data(Model::BeforeRole).toString(), rowFont(12), 130),
                                textHeight(index.data(Model::AfterRole).toString(), rowFont(13, QFont::Medium), 155)});
    return QSize(width, std::max(61, height + 22) + (index.data(Model::LastRole).toBool() ? 17 : 0));
  };
  auto* view = new RecyclingListView(Model::KindRole, std::move(factory), std::move(bind), std::move(measure), parent);
  commands->setParent(view->viewport());
  commands->hide();
  view->setObjectName(QStringLiteral("applyChangesList"));
  view->setModel(&model);
  return view;
}
}  // namespace uwf::ui
