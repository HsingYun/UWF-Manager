// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>

namespace uwf::app {

// Authenticated UI-to-LocalSystem connection. The pipe carries one-shot
// PRESHUTDOWN coordination tokens, not UWF commit tasks.
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
