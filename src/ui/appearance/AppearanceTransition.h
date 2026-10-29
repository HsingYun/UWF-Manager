// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <functional>
class QObject;

namespace uwf::ui {
// One atomic display update followed by a short client-area crossfade. Nested
// changes share the transaction; editors, native frames and input stay alive.
class AppearanceTransition final {
 public:
  static void run(const std::function<void()>& update, bool animate = true);
  static void request(QObject* owner, std::function<void()> update, std::function<bool()> needsPaint = {});
};
}  // namespace uwf::ui
