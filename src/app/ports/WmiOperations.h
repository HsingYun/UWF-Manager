// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <stop_token>

#include "app/wmi/WmiTypes.h"

namespace uwf::app::wmi {

// 领域 API 依赖的 WMI 操作契约。它描述调用语义而不暴露 COM 连接细节，使
// UWF 领域层可以用于其它 WMI transport（例如代理进程或离线验证器）。
// WmiSession 是当前 Windows/COM 实现。
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
  virtual void invokeMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const = 0;
  [[nodiscard]] virtual WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const = 0;
  // Wait until the provider completes or stop is requested, without a local
  // deadline. Provider and transport failures still propagate to the caller.
  [[nodiscard]] virtual WmiMethodOutput callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                                                                 std::stop_token stopToken) const = 0;
  virtual void putInstance(const std::string& className, const WmiRow& props, WmiPutMode mode) const = 0;
};

}  // namespace uwf::app::wmi
