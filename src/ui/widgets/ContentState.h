// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QIcon>
#include <QPointer>
#include <QWidget>

#include "design/DesignTokens.h"

class QProgressBar;
class QPushButton;
class QVBoxLayout;

namespace uwf::ui {
class BusyIndicator;
class IconView;
class TextLabel;
class ContentState final : public QWidget {
  Q_OBJECT
 public:
  // SelfManaged is for fixed viewports such as the main workspace and split panes.
  // HostManaged leaves scrolling to an enclosing page and contributes its complete wrapped height.
  enum class ScrollMode { SelfManaged, HostManaged };
  explicit ContentState(QWidget* parent = nullptr, ScrollMode mode = ScrollMode::SelfManaged);
  void setText(const QString& title, const QString& description);
  void setIcon(const QIcon& icon, design::Color color = design::Color::Faint);
  void setLoading(bool loading);
  void setAction(const QString& text, const QIcon& icon = {});
  void setDetails(QWidget* details);
  [[nodiscard]] bool hasHeightForWidth() const override;
  [[nodiscard]] int heightForWidth(int width) const override;

 signals:
  void actionRequested();

 private:
  IconView* m_icon;
  BusyIndicator* m_spinner;
  TextLabel* m_title;
  TextLabel* m_description;
  QWidget* m_progressHost;
  QProgressBar* m_progress;
  QPushButton* m_action;
  ScrollMode m_scrollMode;
  QWidget* m_layoutHost = nullptr;
  QVBoxLayout* m_layout;
  QPointer<QWidget> m_details;
  QPointer<QWidget> m_detailsRow;
};
}  // namespace uwf::ui
