// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QMargins>
#include <QTextEdit>
namespace uwf::ui {
class CodeEditor final : public QTextEdit {
  Q_OBJECT
  Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
 public:
  explicit CodeEditor(QWidget* parent = nullptr);
  void paintLineNumbers(QPaintEvent* event);
  [[nodiscard]] int blockCount() const;
  void setLineNumbersVisible(bool visible);
  void setLineHeight(qreal pixels);
  void setContentMargins(const QMargins& margins);
  void setPlaceholderText(const QString& text);
  [[nodiscard]] QString placeholderText() const { return m_placeholder; }

 public slots:
  void setPlainText(const QString& text);
  void clear();

 signals:
  void placeholderTextChanged(const QString& text);

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void insertFromMimeData(const QMimeData* source) override;
  void changeEvent(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  void updateGutter();
  void resetLineFormat();
  void normalizeLineFormats(int position, int added);
  bool m_formatting = false;
  QWidget* m_numbers;
  bool m_lineNumbersVisible = true;
  qreal m_lineHeight = 28;
  QMargins m_contentMargins{18, 16, 18, 16};
  QString m_placeholder;
};
}  // namespace uwf::ui
