// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <memory>

#include "WmiRuntime.h"

namespace uwf::windows {

struct WmiThreadContext;

class WmiSession final : public app::wmi::WmiOperations {
 public:
  ~WmiSession() override;
  [[nodiscard]] std::function<app::wmi::WmiOperations&()> threadAccessor() override;
  WmiSession(const WmiSession&) = delete;
  WmiSession& operator=(const WmiSession&) = delete;

  // 确保当前固定 namespace 已连接；失效代理的释放与重建完全由 session 内部
  // 管理，调用方不参与连接生命周期。
  void ensureConnected() const override;

  // 执行一条 WQL 查询，返回每一行的非系统属性集合。若 provider 为结果对象
  // 提供了非空 __PATH，则一并返回；投影查询的 __PATH 合法地可能为 NULL。
  std::vector<app::wmi::WmiRow> query(const std::string& wql) const override;

  // 查询可被后续方法调用精确定位的实例。通过 WBEM_FLAG_ENSURE_LOCATABLE
  // 请求 WMI 补齐定位信息，并把缺失 __PATH 视为不完整结果。
  std::vector<app::wmi::WmiRow> queryInstances(const std::string& wql) const override;

  // 检查命名空间里是否注册了某个类。连接、权限和协议失败抛出异常，绝不
  // 编码成“未知”或误判为“不存在”。
  [[nodiscard]] app::wmi::WmiClassStatus classStatus(const std::string& className) const override;

  // 精确读取一个实例或类对象；读取失败及属性枚举不完整均抛出异常。
  [[nodiscard]] app::wmi::WmiRow getObject(const std::string& objectPath) const override;

  // 调用写方法。objectPath 可以是：
  //   - 类名（单例或 static 方法）
  //   - 实例的 __PATH（推荐；精确定位一行）
  //   - 相对路径 "Class.Key=Val,..."（不带命名空间，如 ensureNextSessionEntry 构造的）
  // inputs 中的 WmiValue 会根据 kind 转成对应 VARIANT 写入 in-params。
  // 成功表示同步调用及 provider ReturnValue 均成功。持久配置必须由领域层重读
  // 可观测状态确认；关机、重启、commit 这类没有通用 WMI 后置条件的一次性命令，
  // 则以 provider 的同步结果为基础，并由掌握领域状态的调用边界在可观测时补充
  // 确认（例如删除 commit 在 UI 批处理层重查目标是否仍存在）。
  void invokeMethod(const std::string& objectPath, const std::string& methodName, const uwf::app::wmi::WmiRow& inputs = {}) const override;
  // 只读方法允许在明确的 RPC/WMI 连接故障后重建连接并重试一次。写方法必须走
  // invokeMethod()，绝不能自动重放。
  [[nodiscard]] app::wmi::WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName,
                                                         const app::wmi::WmiRow& inputs = {}) const override;
  // 可取消的只读方法使用 WMI 原生异步调用；stop 请求会在发起调用的同一线程
  // 执行 CancelAsyncCall，不依赖跨线程 CoCancelCall 的时序窗口。
  [[nodiscard]] app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const app::wmi::WmiRow& inputs,
                                                                   std::stop_token stopToken) const override;

  // 创建和更新必须由调用方明确选择，禁止用 CREATE_OR_UPDATE 模糊并发语义。
  void putInstance(const std::string& className, const uwf::app::wmi::WmiRow& props, uwf::app::wmi::WmiPutMode mode) const override;

 private:
  enum class QueryMode { Projection, LocatableInstances };
  friend struct WmiThreadContext;
  // 只允许 WmiThreadContext 创建两个固定 namespace session，避免调用方另建
  // 短生命周期连接或在运行期改绑 namespace。
  explicit WmiSession(std::string namespacePath);
  std::vector<app::wmi::WmiRow> executeQuery(const std::string& wql, QueryMode mode) const;
  [[nodiscard]] app::wmi::WmiMethodOutput executeReadMethodOnce(const std::string& objectPath, const std::string& methodName,
                                                                const app::wmi::WmiRow& inputs) const;
  void ensureConnectedWithinOperation() const;
  void reconnectWithinOperation() const;
  void invalidate() const;

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace uwf::windows
