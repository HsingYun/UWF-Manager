// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "infrastructure/windows/management/wmi/WmiRuntime.h"

namespace uwf::windows {

struct WmiThreadContext;

class WmiSession final : public windows::wmi::WmiOperations {
 public:
  ~WmiSession() override;
  [[nodiscard]] std::function<windows::wmi::WmiOperations&()> threadAccessor() override;
  WmiSession(const WmiSession&) = delete;
  WmiSession& operator=(const WmiSession&) = delete;

  // The session owns connection recovery for its fixed namespace.
  void ensureConnected() const override;

  // Returns non-system properties and __PATH when supplied. Projection rows
  // may legitimately have no __PATH.
  std::vector<windows::wmi::WmiRow> query(const std::string& wql) const override;

  // Requests locatable instances; a missing __PATH is a protocol failure.
  std::vector<windows::wmi::WmiRow> queryInstances(const std::string& wql) const override;

  // Connection, access and protocol failures throw rather than imply absence.
  [[nodiscard]] windows::wmi::WmiClassStatus classStatus(const std::string& className) const override;

  [[nodiscard]] windows::wmi::WmiRow getObject(const std::string& objectPath) const override;

  // Accepts a class name, instance __PATH or relative keyed object path.
  // Success requires both synchronous completion and a successful ReturnValue;
  // callers interpret other outputs. Dispatched writes are never replayed, and
  // matching readback cannot establish that a failed transition completed.
  windows::wmi::WmiMethodOutput invokeMethod(const std::string& objectPath, const std::string& methodName,
                                             const uwf::windows::wmi::WmiRow& inputs = {}) const override;
  // Reconnects and retries once after a recognized transport failure. Mutating
  // methods must use invokeMethod instead.
  [[nodiscard]] windows::wmi::WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName,
                                                             const windows::wmi::WmiRow& inputs = {}) const override;
  // Uses native async WMI; the invoking thread cancels on stop. There is no
  // local deadline for provider completion.
  [[nodiscard]] windows::wmi::WmiMethodOutput callMethodReadOnce(const std::string& objectPath, const std::string& methodName,
                                                                 const windows::wmi::WmiRow& inputs, std::stop_token stopToken) const override;

  // Explicit create-only/update-only mode preserves concurrency semantics.
  void putInstance(const std::string& className, const uwf::windows::wmi::WmiRow& props, uwf::windows::wmi::WmiPutMode mode) const override;

 private:
  enum class QueryMode { Projection, LocatableInstances };
  friend struct WmiThreadContext;
  // WmiThreadContext owns the two fixed-namespace sessions for each thread.
  explicit WmiSession(std::string namespacePath);
  std::vector<windows::wmi::WmiRow> executeQuery(const std::string& wql, QueryMode mode) const;
  [[nodiscard]] windows::wmi::WmiMethodOutput executeReadMethodOnce(const std::string& objectPath, const std::string& methodName,
                                                                    const windows::wmi::WmiRow& inputs) const;
  void ensureConnectedWithinOperation() const;
  void reconnectWithinOperation() const;
  void invalidate() const;

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace uwf::windows
