// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QDialog>
#include <QMargins>
#include <optional>

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
  struct HeaderMetrics {
    QMargins margins;
    int titleFontSize = 0;  // Zero keeps the size selected by HeaderStyle.
    qreal titleLineHeight = 0;
  };
  [[nodiscard]] QPushButton* closeButton() const { return m_close; }
  void setDescription(const QString& text);
  void setHeaderIcon(const QIcon& icon);
  void setHeaderStyle(HeaderStyle style);
  void setHeaderMetrics(HeaderMetrics metrics);
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
  void bindRejectionButton(QAbstractButton* button);
  void changeEvent(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
  void showEvent(QShowEvent* event) override;
  bool nativeEvent(const QByteArray& type, void* message, qintptr* result) override;

 private:
  friend class DialogLayout;
  void updateHeaderMetrics();
  void updateTitleFont();
  void showSystemMenu(const QPoint& position);

  Kind m_kind;
  bool m_rejectionEnabled = true;
  HeaderStyle m_headerStyle = HeaderStyle::Standard;
  std::optional<HeaderMetrics> m_headerMetrics;
  QWidget* m_header;
  QLabel* m_eyebrow;
  QLabel* m_title;
  QLabel* m_description;
  QWidget* m_headerIcon;
  IconView* m_headerGlyph;
  QPushButton* m_close;
};
}  // namespace uwf::ui
