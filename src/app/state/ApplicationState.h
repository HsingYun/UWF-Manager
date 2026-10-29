// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <functional>
#include <stop_token>
#include <vector>

#include "app/api/UwfCapability.h"
#include "core/UwfModel.h"

namespace uwf::app {

struct ApplicationState {
  std::vector<core::VolumeInfo> volumes;
  core::UwfSnapshot snapshot;
};

// 主窗口只依赖“读取一份完整应用状态”的能力，不关心数据来自本机 WMI、远程
// 代理还是离线快照。实现必须完整返回磁盘与 UWF 快照，或在失败时抛出异常；
// 在调用线程准备一个拥有全部输入的读取任务，在工作线程运行；任务不得捕获
// source 或 GUI 对象的裸引用。状态会话只提交完整结果，失败时保留上一份快照。
class ApplicationStateSource {
 public:
  using ReadOperation = std::function<ApplicationState(std::stop_token)>;
  virtual ~ApplicationStateSource() = default;
  [[nodiscard]] virtual ReadOperation readOperation(UwfCapability capability) = 0;
};

}  // namespace uwf::app
