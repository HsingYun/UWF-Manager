/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <memory>

#include "TaskbarLayoutStrategy.h"

namespace uwf::ui {

class Win11TaskbarLayoutStrategyTestAccess;

// 将 Hub 原生窗口注入 Win11 主任务栏。任何本地 attachment 不变量异常都触发
// 完整平台窗口重建；不在半失效 HWND 上继续做增量修复。
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
  // 仅供测试编译单元中的 Win11TaskbarLayoutStrategyTestAccess 使用；不在生产库中提供种植入口。
  friend class Win11TaskbarLayoutStrategyTestAccess;
  // Parent 提交不完整时与 detach() 共用严格回滚。
  [[nodiscard]] AttachResult abortIncompleteParentCommit();
  void recordVerificationDiagnostic(VerificationResult result, const char* reason) const;
  class AttachTransactionImpl;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace uwf::ui
