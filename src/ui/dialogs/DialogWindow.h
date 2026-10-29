// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QDialog>

class QLabel;
class QPushButton;
class QAbstractButton;

namespace uwf::ui {
class DialogLayout;
class IconView;

class DialogWindow : public QDialog {
  Q_OBJECT
 public:
  enum class HeaderStyle { Standard, Power, Message };
  [[nodiscard]] QPushButton* closeButton() const { return m_close; }
  void setDescription(const QString& text);
  void setHeaderIcon(const QIcon& icon);
  void setHeaderStyle(HeaderStyle style);
  [[nodiscard]] bool rejectionEnabled() const { return m_rejectionEnabled; }
  void setRejectionEnabled(bool enabled);
  void reject() override final;
  void done(int result) override final;

 signals:
  void rejectionEnabledChanged(bool enabled);

 protected:
  enum class Kind { Panel, Information };
  explicit DialogWindow(Kind kind, QWidget* parent);
  // A cancellation request may need to finish asynchronously before closing.
  virtual bool requestRejection();
  void changeEvent(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
  void showEvent(QShowEvent* event) override;
  bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override;

 private:
  friend class DialogLayout;
  void updateTitleFont();
  void showSystemMenu(const QPoint& position);
  void bindRejectionButton(QAbstractButton* button);

  Kind m_kind;
  bool m_rejectionEnabled = true;
  HeaderStyle m_headerStyle = HeaderStyle::Standard;
  QWidget* m_header;
  QLabel* m_eyebrow;
  QLabel* m_title;
  QLabel* m_description;
  QWidget* m_headerIcon;
  IconView* m_headerGlyph;
  QPushButton* m_close;
};
}  // namespace uwf::ui
