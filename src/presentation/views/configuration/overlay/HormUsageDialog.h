// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include "presentation/dialogs/PanelDialog.h"

class QResizeEvent;

namespace uwf::ui {
class DialogLayout;

// Read-only instructions. Commands are selectable text, never executable actions.
class HormUsageDialog final : public PanelDialog {
  Q_OBJECT
 public:
  explicit HormUsageDialog(QWidget* parent = nullptr);

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void updateResponsiveLayout();
  DialogLayout* m_frame = nullptr;
  int m_layoutProfile = -1;
};
}  // namespace uwf::ui
