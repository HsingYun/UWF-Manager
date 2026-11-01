// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QComboBox>
#include <QPointer>
class QListView;
namespace uwf::ui {
class SelectPopup;
class SelectBox final : public QComboBox {
  Q_OBJECT
 public:
  explicit SelectBox(QWidget* parent = nullptr);
  ~SelectBox() override;
  void showPopup() override;
  void hidePopup() override;

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void changeEvent(QEvent* event) override;

 private:
  void acceptIndex(const QModelIndex& index);
  QPointer<SelectPopup> m_popup;
  QListView* m_options = nullptr;
  bool m_pointerDown = false;
  bool m_hovered = false;
};
}  // namespace uwf::ui
