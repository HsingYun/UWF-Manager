// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <QStringList>
#include <cstdint>
#include <functional>
#include <memory>

namespace uwf::windows {
class MtaExecutor;
struct DmCspReply {
  std::uint32_t hresult = 0;
  QString xml;
  QString extendedError;
  bool invoked = true;
};

// Generic DM transport: the caller owns the document, CSP filter and protocol
// interpretation. This layer only manages DLL/COM lifetime and native diagnostics.
class DmCspTransport {
 public:
  virtual ~DmCspTransport() = default;
  [[nodiscard]] virtual bool available() { return true; }
  [[nodiscard]] virtual DmCspReply process(const QString& xml, const QStringList& allowedCspNames) = 0;
};
[[nodiscard]] std::shared_ptr<DmCspTransport> createSystemDmCspTransport();
// The factory, calls, and final release run within the supplied MTA pool. The
// synchronous port leaves scheduling and delivery to its application caller.
[[nodiscard]] std::shared_ptr<DmCspTransport> createMtaDmCspTransport(std::function<std::shared_ptr<DmCspTransport>()> factory,
                                                                      std::shared_ptr<MtaExecutor> executor);
}  // namespace uwf::windows
