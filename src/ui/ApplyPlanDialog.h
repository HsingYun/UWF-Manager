#pragma once

// "审阅并应用"对话框。
//
// 把 GlobalStatusPanel 与各个 DiskTab 上累积的待应用变更收集成
// core::PendingChanges，以 uwfmgr 命令的形式展示（待应用变更段 + 当前会话
// 配置段），允许导出命令脚本，并在用户二次确认后逐项写入 WMI。每一步单独
// 收集错误，不因单点失败终止其它写入。应用完成后发 applied() 信号，让宿主
// 窗口重新读取快照刷新 UI。

#include <QDialog>
#include <QPointer>
#include <QVector>
#include <string>
#include <vector>

#include "../core/UwfModel.h"
#include "../uwf/api/UwfFilter.h"
#include "../uwf/api/UwfOverlay.h"
#include "../uwf/api/UwfOverlayConfig.h"
#include "../uwf/api/UwfRegistryFilter.h"
#include "../uwf/api/UwfVolume.h"
#include "../uwf/wmi/WmiClient.h"

namespace uwf::ui {

class GlobalStatusPanel;
class DiskTab;

class ApplyPlanDialog : public QDialog {
  Q_OBJECT
 public:
  // global / diskTabs 提供待应用变更的来源；snapshot 是当前 UWF 快照
  // （写入时需要它判断 filter 是否已启用）；writeSession 是写操作共用的
  // WMI 会话。三者的所有权都不转移——调用方需保证其生命周期覆盖本对话框。
  ApplyPlanDialog(GlobalStatusPanel* global, const QVector<QPointer<DiskTab>>& diskTabs, const core::UwfSnapshot& snapshot, WmiSession& writeSession,
                  QWidget* parent = nullptr);

 signals:
  // 用户点击"应用"并完成写入后发出。宿主窗口应据此重新读取快照。
  void applied();

 private:
  // 一条变更或快照配置：comment 是中文说明（渲染成 ":: 注释"行），cmd 是
  // 对应的 uwfmgr 命令；cmd 为空表示该项无命令行对应，只展示注释。
  struct Cmd {
    std::string comment;
    std::string cmd;
  };

  WmiSession& m_session;
  const core::UwfSnapshot& m_snapshot;
  UwfFilter m_filter;
  UwfOverlay m_overlay;
  UwfOverlayConfig m_overlayConfig;
  UwfVolume m_volume;
  UwfRegistryFilter m_registry;

  core::PendingChanges m_changes;
  std::vector<Cmd> m_changeCmds;
  std::vector<Cmd> m_snapshotCmds;
};

}  // namespace uwf::ui
