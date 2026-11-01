// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QTabWidget>
#include <memory>
namespace uwf::ui {
class TabView final : public QTabWidget {
 public:
  enum class Kind { Pages, Volumes, Entries };
  explicit TabView(QWidget* parent = nullptr);
  void setKind(Kind kind);
  void setTabCount(int index, int count);
  void setTabIndicator(int index, const QString& iconResource, const QString& hint);
  void setCompact(bool compact);
  void setMeasureCurrentPage(bool enabled);
  [[nodiscard]] QSize minimumSizeHint() const override;
  [[nodiscard]] bool hasHeightForWidth() const override;
  [[nodiscard]] int heightForWidth(int width) const override;

 protected:
  void paintEvent(QPaintEvent* event) override;
  void tabInserted(int index) override;

 private:
  bool m_measureCurrentPage = false;
  std::shared_ptr<bool> m_preserveScroll = std::make_shared<bool>(false);
};
}  // namespace uwf::ui
