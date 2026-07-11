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

// 仅匹配包含 Win11 XAML composition bridge 的原生任务栏。任何必要锚点或
// 几何信息缺失都视为不可用，交由 Hub 回退，而不是使用经验魔数猜测位置。
class Win11TaskbarLayoutStrategy final : public TaskbarLayoutStrategy {
 public:
  Win11TaskbarLayoutStrategy();
  ~Win11TaskbarLayoutStrategy() override;

  [[nodiscard]] int priority() const override { return 200; }
  [[nodiscard]] bool available() const override;
  bool attach(WId window, const QSize& logicalSize) override;
  [[nodiscard]] bool verify(WId window) const override;
  void detach(WId window) override;
  void invalidate() override;

 private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace uwf::ui
