// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>

namespace uwf::app {

// UI 进程与 LocalSystem 服务之间的身份连接边界。管道不承载 UWF 提交任务；
// 除维持双方身份外，只传递安全电源流程的一次性 PRESHUTDOWN 令牌状态。
class EnhancedModeAgentConnection : public QObject {
  Q_OBJECT
 public:
  explicit EnhancedModeAgentConnection(QObject* parent = nullptr) : QObject(parent) {}
  ~EnhancedModeAgentConnection() override = default;

  virtual void start() = 0;
  virtual void stop() = 0;
  [[nodiscard]] virtual bool running() const = 0;
  [[nodiscard]] virtual bool markPreshutdownCommitHandled() = 0;
  [[nodiscard]] virtual bool requirePreshutdownCommit() = 0;

 signals:
  void connectionStateChanged(bool connected);
};

}  // namespace uwf::app
