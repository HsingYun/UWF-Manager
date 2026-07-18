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

// 把 MainWindow 的 4 个 commit* 槽内部的 batch 提交流程收拢成对象——4 个入口
// 形态相似：precondition 对话框 → 卷 / 注册表行查找 → 排除冲突检查 → 目标
// 枚举 → runCommitBatch。所有依赖（写会话 / 快照 / Usage 定时器 / dialog
// parent）由调用方注入，CommitDispatcher 自身不持有它们的生命周期。

#include <QString>

#include "../core/UwfModel.h"
#include "../uwf/api/UwfRegistryFilter.h"
#include "../uwf/api/UwfVolume.h"
#include "../uwf/wmi/WmiClient.h"

class QTimer;
class QWidget;

namespace uwf::ui {

class CommitDispatcher {
 public:
  // session    用于 UWF_Volume / UWF_RegistryFilter 上的写操作；
  // snapshot   排除列表预校验的来源（取当前会话）；
  // usageTimer 每次 commit 期间挂起、离开作用域恢复，防止 5s 占用刷新在
  //            QProgressDialog::setValue 内部 processEvents 半途回调时对同一
  //            session 发起重入 WMI 调用；
  // parent     给所有模态对话框（confirmCommit / warning / QProgressDialog）
  //            做 QWidget 父对象。
  // 所有引用 / 指针的生命周期须覆盖本对象——CommitDispatcher 只引用，不拥有。
  CommitDispatcher(WmiOperations& session, const core::UwfSnapshot& snapshot, QTimer* usageTimer, QWidget* parent);

  // 与 MainWindow 同名槽语义保持一致——拆分前后行为按字节等价：
  void commitFilePath(const QString& path);
  void commitFileDeletionPath(const QString& path);
  // valueName 空串 = 递归整棵键子树；非空 = 单个命名值。(Default) 在 picker
  // 层恒禁选，故 valueName 非空必为真实值名，空串无歧义地表示"整键递归"。
  void commitRegistryKey(const QString& key, const QString& valueName);
  void commitRegistryDeletionKey(const QString& key, const QString& valueName);

 private:
  WmiOperations& m_session;
  const core::UwfSnapshot& m_snapshot;
  QTimer* m_usageTimer;
  QWidget* m_parent;
  api::UwfVolume m_volume;
  api::UwfRegistryFilter m_registry;
};

}  // namespace uwf::ui
