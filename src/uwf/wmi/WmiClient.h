/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// WMI 客户端抽象：不依赖 Qt。
//
// WmiValue     —— 类似 QVariant：把 VARIANT 的几种常用类型抹成统一的
//                 {Bool/Int/UInt/Double/String}。
// WmiRow       —— map<属性名, WmiValue>；代表一行查询结果或一个方法的
//                 标量输出参数集合。
// WmiMethodOutput —— 成功的 ExecMethod 输出。失败不编码进返回值，统一抛出
//                 WmiException 的具体子类。
// WmiSession   —— 持有固定 namespace 的 IWbemLocator / IWbemServices，
//                 连接失效后可重建；读操作可安全重试，写操作不重放。

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <vector>

namespace uwf {

struct WmiThreadContext;

class WmiValueConversionError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class WmiValue {
 public:
  enum class Kind { None, Bool, Int, UInt, Double, String };

  WmiValue() = default;
  static WmiValue fromBool(bool v);
  static WmiValue fromInt(int64_t v);
  static WmiValue fromUInt(uint64_t v);
  static WmiValue fromDouble(double v);
  static WmiValue fromString(std::string v);

  [[nodiscard]] Kind kind() const { return m_kind; }
  [[nodiscard]] bool isValid() const { return m_kind != Kind::None; }

  // 各种 to*() 都会做可验证的类型转换：比如 string "1" 可转为整数。
  // 无值、格式错误或越界抛出 WmiValueConversionError，不用 ok/def
  // 参数把数据损坏静默折叠成业务值。
  [[nodiscard]] bool toBool() const;
  [[nodiscard]] int32_t toInt() const;
  [[nodiscard]] uint32_t toUInt() const;
  [[nodiscard]] int64_t toInt64() const;
  [[nodiscard]] uint64_t toULongLong() const;
  [[nodiscard]] double toDouble() const;
  [[nodiscard]] std::string toString() const;

 private:
  Kind m_kind = Kind::None;
  bool m_bool = false;
  int64_t m_int = 0;
  uint64_t m_uint = 0;
  double m_double = 0.0;
  std::string m_string;
};

using WmiRow = std::map<std::string, WmiValue>;

struct WmiMethodOutput {
  WmiRow values;
  std::map<std::string, std::vector<WmiRow>> arrays;
};

enum class WmiClassStatus { Present, Missing };
enum class WmiPutMode { CreateOnly, UpdateOnly };

// 领域 API 依赖的 WMI 操作契约。它描述调用语义而不暴露 COM 连接细节，使
// UWF 领域层可以用于其它 WMI transport（例如代理进程或离线验证器）。
// WmiSession 是当前 Windows/COM 实现。
class WmiOperations {
 public:
  virtual ~WmiOperations() = default;
  virtual void ensureConnected() const = 0;
  [[nodiscard]] virtual std::vector<WmiRow> query(const std::string& wql) const = 0;
  [[nodiscard]] virtual std::vector<WmiRow> queryInstances(const std::string& wql) const = 0;
  [[nodiscard]] virtual WmiClassStatus classStatus(const std::string& className) const = 0;
  [[nodiscard]] virtual WmiRow getObject(const std::string& objectPath) const = 0;
  virtual void invokeMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const = 0;
  [[nodiscard]] virtual WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const = 0;
  [[nodiscard]] virtual WmiMethodOutput callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                                                                 std::stop_token stopToken) const = 0;
  virtual void putInstance(const std::string& className, const WmiRow& props, WmiPutMode mode) const = 0;
};

class WmiSession final : public WmiOperations {
 public:
  ~WmiSession() override;
  WmiSession(const WmiSession&) = delete;
  WmiSession& operator=(const WmiSession&) = delete;

  // 确保当前固定 namespace 已连接；失效代理的释放与重建完全由 session 内部
  // 管理，调用方不参与连接生命周期。
  void ensureConnected() const override;

  // 执行一条 WQL 查询，返回每一行的非系统属性集合。若 provider 为结果对象
  // 提供了非空 __PATH，则一并返回；投影查询的 __PATH 合法地可能为 NULL。
  std::vector<WmiRow> query(const std::string& wql) const override;

  // 查询可被后续方法调用精确定位的实例。通过 WBEM_FLAG_ENSURE_LOCATABLE
  // 请求 WMI 补齐定位信息，并把缺失 __PATH 视为不完整结果。
  std::vector<WmiRow> queryInstances(const std::string& wql) const override;

  // 检查命名空间里是否注册了某个类。连接、权限和协议失败抛出异常，绝不
  // 编码成“未知”或误判为“不存在”。
  [[nodiscard]] WmiClassStatus classStatus(const std::string& className) const override;

  // 精确读取一个实例或类对象；读取失败及属性枚举不完整均抛出异常。
  [[nodiscard]] WmiRow getObject(const std::string& objectPath) const override;

  // 调用写方法。objectPath 可以是：
  //   - 类名（单例或 static 方法）
  //   - 实例的 __PATH（推荐；精确定位一行）
  //   - 相对路径 "Class.Key=Val,..."（不带命名空间，如 ensureNextSessionEntry 构造的）
  // inputs 中的 WmiValue 会根据 kind 转成对应 VARIANT 写入 in-params。
  // 成功表示同步调用及 provider ReturnValue 均成功。持久配置必须由领域层重读
  // 可观测状态确认；关机、重启、commit 这类没有通用 WMI 后置条件的一次性命令，
  // 则以 provider 的同步结果为基础，并由掌握领域状态的调用边界在可观测时补充
  // 确认（例如删除 commit 在 UI 批处理层重查目标是否仍存在）。
  void invokeMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const override;
  // 只读方法允许在明确的 RPC/WMI 连接故障后重建连接并重试一次。写方法必须走
  // invokeMethod()，绝不能自动重放。
  [[nodiscard]] WmiMethodOutput callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const override;
  // 可取消的只读方法使用 WMI 原生异步调用；stop 请求会在发起调用的同一线程
  // 执行 CancelAsyncCall，不依赖跨线程 CoCancelCall 的时序窗口。
  [[nodiscard]] WmiMethodOutput callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                                                         std::stop_token stopToken) const override;

  // 创建和更新必须由调用方明确选择，禁止用 CREATE_OR_UPDATE 模糊并发语义。
  void putInstance(const std::string& className, const WmiRow& props, WmiPutMode mode) const override;

 private:
  enum class QueryMode { Projection, LocatableInstances };
  friend struct WmiThreadContext;
  // 只允许 WmiThreadContext 创建两个固定 namespace session，避免调用方另建
  // 短生命周期连接或在运行期改绑 namespace。
  explicit WmiSession(std::string namespacePath);
  std::vector<WmiRow> executeQuery(const std::string& wql, QueryMode mode) const;
  [[nodiscard]] WmiMethodOutput executeReadMethodOnce(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs) const;
  void ensureConnectedWithinOperation() const;
  void reconnectWithinOperation() const;
  void invalidate() const;

  struct Impl;
  std::unique_ptr<Impl> d;
};

// 每个线程各自拥有两个固定 namespace 的长生命周期 session。首次访问会初始化
// 当前线程的 COM apartment；线程退出时先释放 session，再成对 CoUninitialize。
// 返回的引用不得跨线程传递；session 会在所有操作入口校验其创建线程。
WmiSession& embeddedWmiSession();
WmiSession& cimv2WmiSession();

// 在启动线程显式建立 COM apartment 与进程安全策略。失败抛出异常，由进程或
// 线程入口统一决定日志和用户提示策略。
void initializeWmiRuntime();

}  // namespace uwf
