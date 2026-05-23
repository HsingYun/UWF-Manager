#pragma once

// 把 ImportDialog 解析出的 UwfmgrCommand 列表"应用"成 MainWindow 各面板的
// pending 变更，并产出每条命令的 ImportReportRow。从 MainWindow::showImport
// 里拆出来——applier 自身不持有窗体，只读 / 写 global 面板与各 DiskTab，便于
// 单元测试和后续在 ApplyPlanDialog 之外的入口复用同一套语义。

#include <QList>
#include <QPointer>
#include <QVector>

#include "../uwf/api/UwfmgrCli.h"
#include "ImportDialog.h"

namespace uwf::ui {

class DiskTab;
class GlobalStatusPanel;

// applier 返回值：每条命令对应一行报告（含解析失败 / within-batch dedup / state
// no-op）。整批跑完会调一次 global->finishImport() 收紧 spinbox 约束链。
// global 可为空（极少见，比如 buildUi 还没跑完时被外部触发）；diskTabs 的元素
// 可能含已被回收的弱指针，遍历时按 QPointer 习惯过滤。
QList<ImportReportRow> applyImportCommands(const QList<api::UwfmgrCommand>& cmds, GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs);

}  // namespace uwf::ui
