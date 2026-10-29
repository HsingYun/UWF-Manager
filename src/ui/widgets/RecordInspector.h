// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QIcon>
#include <QLayout>

#include "Surface.h"

class QHBoxLayout;
class QPushButton;
namespace uwf::ui {
class ContentHintBox;
class TextLabel;
class RecordInspector final : public Surface {
 public:
  explicit RecordInspector(const QString& title, int detailLines, QWidget* parent = nullptr);
  void setTitle(const QString& title);
  void setMetadata(const QString& text);
  void setNote(const QString& text);
  void setText(const QString& text);
  [[nodiscard]] bool hasHeightForWidth() const override { return true; }
  [[nodiscard]] int heightForWidth(int width) const override { return layout() ? layout()->totalHeightForWidth(width) : Surface::heightForWidth(width); }
  [[nodiscard]] ContentHintBox* textView() const { return m_text; }
  [[nodiscard]] QPushButton* addAction(const QString& text, const QIcon& icon = {});

 private:
  TextLabel* m_title;
  TextLabel* m_metadata;
  TextLabel* m_note;
  ContentHintBox* m_text;
  QHBoxLayout* m_actions;
};
}  // namespace uwf::ui
