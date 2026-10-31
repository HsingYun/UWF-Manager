// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>
#include <vector>

#include "app/api/EnhancedModeStatus.h"
#include "app/api/RegistryCommitTypes.h"

namespace uwf::app {

// Windows 服务及其注册表工件共同构成明确的系统边界。UI 与业务编排只依赖
// 这个接口，不能在注入内存实现后仍绕过它访问真实 SCM 或 HKLM。
class EnhancedModeServiceControl {
 public:
  virtual ~EnhancedModeServiceControl() = default;
  [[nodiscard]] virtual EnhancedModeStatus query() const = 0;
  virtual void installAndStart(const QString& description) = 0;
  virtual void start() = 0;
  [[nodiscard]] virtual std::vector<RegistryCommitTarget> planRegistryCommit() const = 0;
  [[nodiscard]] virtual std::vector<RegistryCommitTarget> planRegistryDeletion() const = 0;
  [[nodiscard]] virtual bool registryTargetExists(const RegistryCommitTarget& target) const = 0;
  virtual void deleteRegistryRemnants() = 0;
  // SCM 只有在服务停止且最后一个服务句柄关闭后才真正删除服务项。调用方在
  // 返回的屏障存活期间提交服务注册表键的递归删除，随后销毁屏障完成删除。
  class DeletionBarrier {
   public:
    virtual ~DeletionBarrier() = default;
  };
  enum class RemovalState {
    Absent,
    AlreadyMarkedForDeletion,
    MarkedForDeletion,
  };
  struct Removal {
    RemovalState state = RemovalState::Absent;
    std::unique_ptr<DeletionBarrier> barrier;
  };
  [[nodiscard]] virtual Removal stopAndMarkForDeletion() = 0;
};

}  // namespace uwf::app
