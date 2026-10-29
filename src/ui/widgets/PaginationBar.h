// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QWidget>
#include "TextLabel.h"
namespace uwf::presentation {
class PageProxyModel;
}
namespace uwf::ui {
class PaginationBar final : public QWidget {
 public:
  explicit PaginationBar(presentation::PageProxyModel& pages, QWidget* parent = nullptr, TextStyle textStyle = TextStyle::Eyebrow);
  void setSummary(const QString& text);

 private:
  TextLabel* m_summary;
};
}  // namespace uwf::ui
