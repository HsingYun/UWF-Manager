// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QWidget>
#include <array>

#include "presentation/controls/text/TextLabel.h"
class QPushButton;
namespace uwf::presentation {
class PageProxyModel;
}
namespace uwf::ui {
class PaginationBar final : public QWidget {
 public:
  explicit PaginationBar(presentation::PageProxyModel& pages, QWidget* parent = nullptr, TextStyle textStyle = TextStyle::Eyebrow);
  void setSummary(const QString& text);

 protected:
  void changeEvent(QEvent* event) override;

 private:
  void retranslate();
  void refreshIcons();
  TextLabel* m_summary;
  std::array<QPushButton*, 4> m_buttons{};
};
}  // namespace uwf::ui
