// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "presentation/views/overlay/taskbar/TaskbarLayoutStrategy.h"

namespace uwf::ui {

class Win11TaskbarLayoutStrategyTestAccess;

// Attaches the HUD to the Windows 11 primary taskbar. Broken attachment
// invariants require platform-window recreation before another transaction.
class Win11TaskbarLayoutStrategy final : public TaskbarLayoutStrategy {
 public:
  Win11TaskbarLayoutStrategy();
  ~Win11TaskbarLayoutStrategy() override;

  [[nodiscard]] bool isCompatible() const override;
  [[nodiscard]] int priority() const override { return 200; }
  [[nodiscard]] std::unique_ptr<AttachTransaction> prepareAttach(QWindow* window, const QSize& logicalSize) override;
  [[nodiscard]] VerificationResult verify(const QWindow* window, WId currentWindowId) const override;
  DetachResult detach() override;
  DetachResult invalidate() override;

 private:
  // Test support supplies controlled observations without a production injection API.
  friend class Win11TaskbarLayoutStrategyTestAccess;

  [[nodiscard]] AttachResult abortIncompleteParentCommit();
  void recordVerificationDiagnostic(VerificationResult result, const char* reason) const;
  class AttachTransactionImpl;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace uwf::ui
