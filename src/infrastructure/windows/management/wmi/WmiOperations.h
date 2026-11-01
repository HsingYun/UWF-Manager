// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <stop_token>

#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiTypes.h"

namespace uwf::windows::wmi {

// Native WMI transport contract. Application use cases depend on semantic
// repositories; WMI adapters use this interface to access COM or a test transport.
class WmiOperations {
 public:
  virtual ~WmiOperations() = default;
  // A worker resolves its own transport. Thread-neutral adapters may share
  // themselves; apartment-bound implementations must return a thread accessor.
  [[nodiscard]] virtual std::function<WmiOperations&()> threadAccessor() {
    return [this]() -> WmiOperations& { return *this; };
  }
  virtual void ensureConnected() const = 0;
  [[nodiscard]] virtual std::vector<WmiRow> query(const std::string& wql) const = 0;
  [[nodiscard]] virtual std::vector<WmiRow> queryInstances(const std::string& wql) const = 0;
  [[nodiscard]] virtual WmiClassStatus classStatus(const std::string& className) const = 0;
  [[nodiscard]] virtual WmiRow getObject(const std::string& objectPath) const = 0;
  virtual WmiMethodOutput invokeMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const = 0;
  [[nodiscard]] virtual WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const = 0;
  // Expensive reads dispatch at most once. Failure carries completion evidence;
  // canceling the client does not establish that provider work has stopped.
  // Legacy transports must opt in rather than silently use a retrying call.
  [[nodiscard]] virtual WmiMethodOutput callMethodReadOnce(const std::string&, const std::string& methodName, const WmiRow&, std::stop_token) const {
    throw WmiProtocolError("invoke single-dispatch WMI method " + methodName, "This transport does not implement single-dispatch method reads");
  }
  virtual void putInstance(const std::string& className, const WmiRow& props, WmiPutMode mode) const = 0;
};

}  // namespace uwf::windows::wmi
