// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPoint>
#include <QSize>
#include <QVBoxLayout>

#include "ui/widgets/FlexibleColumnLayout.h"

class QDialog;
class QDialogButtonBox;
class QFrame;
class QLabel;

namespace uwf::ui {

class VerticalScrollArea;

enum class DialogContent { Scrollable, Form };

// Presentation only: callers retain ownership of action semantics, results,
// modality, cancellation and task lifetime.
class DialogLayout final : public QVBoxLayout {
 public:
  explicit DialogLayout(QDialog* dialog, DialogContent content = DialogContent::Scrollable, QSize initialSize = {}, QSize minimumSize = {});

  [[nodiscard]] FlexibleColumnLayout* body() const { return m_body; }
  [[nodiscard]] VerticalScrollArea* scrollArea() const { return m_scroll; }
  void setActions(QDialogButtonBox* buttons);
  void setActions(QDialogButtonBox* buttons, QLabel* hint);
  void setActions(QDialogButtonBox* buttons, QWidget* leadingContent);
  void setActions(QDialogButtonBox* buttons, QDialogButtonBox* auxiliary);
  void setActions(QDialogButtonBox* buttons, QDialogButtonBox* auxiliary, QWidget* trailingInformation);

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  void fitToContent();
  void updateAdaptiveSize();
  void updateChromeMinimums();
  [[nodiscard]] QRect availableWorkArea() const;
  [[nodiscard]] QSize availableClientSize() const;
  void setActionWidgets(QDialogButtonBox* buttons, QWidget* leading, QWidget* trailingInformation = nullptr);
  [[nodiscard]] int contentHeight() const;
  [[nodiscard]] int chromeHeight() const;

  QDialog* m_dialog;
  QWidget* m_content;
  FlexibleColumnLayout* m_body;
  QFrame* m_footer;
  QWidget* m_header = nullptr;
  VerticalScrollArea* m_scroll = nullptr;
  bool m_fitContent;
  bool m_initialized = false;
  bool m_updateQueued = false;
  bool m_fitting = false;
  int m_initialHeight = 0;
  int m_actionPaddingX;
  int m_actionPaddingY;
  int m_actionHeight;
  QPoint m_center;
  QSize m_minimumSize;
};

}  // namespace uwf::ui
