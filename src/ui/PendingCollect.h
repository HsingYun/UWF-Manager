#pragma once

// 把 GlobalStatusPanel 与各 DiskTab 上累积的待应用变更收集成
// core::PendingChanges。只采集"和基线不同"的字段。ApplyPlanDialog（写入 +
// 预览）与 MainWindow（状态栏计数）共用同一次遍历，避免对同一组 pendingX()
// getter 多处各写一遍、容易漏字段或口径不一致。

#include <QPointer>
#include <QVector>

#include "../core/UwfConfig.h"

namespace uwf::ui {

class GlobalStatusPanel;
class DiskTab;

[[nodiscard]] core::PendingChanges collectPending(const GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs);

}  // namespace uwf::ui
