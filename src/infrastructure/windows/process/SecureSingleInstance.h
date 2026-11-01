// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "application/startup/ApplicationCommand.h"

class QLocalSocket;

namespace uwf::windows {

// Coordinates primary-instance ownership and activation through authenticated IPC.
class SecureSingleInstance final : public QObject {
  Q_OBJECT
 public:
  struct Scope {
    // Empty selects the default scope. Other values are hashed into the pipe
    // name to isolate logical instances without changing peer authentication.
    QString discriminator;
  };

  enum class AcquireResult {
    Primary,
    ForwardedExisting,
    Unprotected,
  };

  explicit SecureSingleInstance(QObject* parent = nullptr);
  explicit SecureSingleInstance(const Scope& scope, QObject* parent = nullptr);
  ~SecureSingleInstance() override;

  SecureSingleInstance(const SecureSingleInstance&) = delete;
  SecureSingleInstance& operator=(const SecureSingleInstance&) = delete;

  // Forwards Activate/EnsureRunning to a trusted peer with a correlated reply,
  // or registers this process. Unprotected means forwarding and listening both
  // failed; the caller may continue without a single-instance guarantee.
  [[nodiscard]] AcquireResult acquire(app::ApplicationCommandKind command = app::ApplicationCommandKind::Activate);
  [[nodiscard]] QString errorString() const;

  // Call after connecting activationRequested; earlier requests are coalesced.
  void enableCommandNotifications();

 signals:
  void activationRequested();

 private:
  class Impl;
  std::unique_ptr<Impl> m_impl;

  void processPendingConnections();
  void processSocket(QLocalSocket* socket);
  void deliverPendingActivation();
};

}  // namespace uwf::windows
