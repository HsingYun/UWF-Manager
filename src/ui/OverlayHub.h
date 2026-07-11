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

#include <QObject>
#include <memory>
#include <optional>
#include <vector>

#include "../core/UwfModel.h"

namespace uwf::ui {

class OverlayHubView;

// 对外唯一可见的 overlay HUD。任务栏视图和桌面浮窗是内部互备实现；调用方
// 只管理 Hub 的可用性、开关和数据，不感知当前实际承载者。
class OverlayHub final : public QObject {
  Q_OBJECT
 public:
  explicit OverlayHub(QObject* parent = nullptr);
  ~OverlayHub() override;

  void registerView(std::unique_ptr<OverlayHubView> view);
  void updateUsage(const core::OverlayRuntime& runtime);
  void setUsageUnavailable();
  void setFilterEnabled(bool enabled);
  void setRequestedVisible(bool visible);
  void hideTemporarily();
  void restoreAfterTemporaryHide();

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool enabled() const;
  [[nodiscard]] bool presented() const;

 signals:
  void showMainWindowRequested();
  void exitApplicationRequested();
  void stateChanged();

 private:
  void reconcilePresentation();
  void hideView(OverlayHubView* view);

  struct ViewEntry {
    std::unique_ptr<OverlayHubView> view;
    bool enabled = true;
  };

  std::vector<ViewEntry> m_views;
  std::optional<core::OverlayRuntime> m_runtime;
  OverlayHubView* m_presentedView = nullptr;
  bool m_filterEnabled = false;
  bool m_newViewsEnabled = true;
  bool m_temporarilyHidden = false;
  bool m_reconciling = false;
};

}  // namespace uwf::ui
