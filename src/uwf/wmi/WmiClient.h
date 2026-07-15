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
// WmiMethodResult —— ExecMethod 的结果：包含 WMI 层是否成功、方法的
//                 UInt32 ReturnValue、以及 out 参数（标量 outParams
//                 和数组类型 outArrays）。
// WmiSession   —— 持有固定 namespace 的 IWbemLocator / IWbemServices，
//                 连接失效后可重建；读操作可安全重试，写操作不重放。

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <vector>

namespace uwf {

// 前向声明：putInstance / deleteInstance 返回 WmiResult，但 WmiResult.h 反过来
// 依赖本文件（用 WmiMethodResult / WmiError）。.cpp 实现处会 include WmiResult.h，
// 调用方也会 include 它，所以 header 这里只需要 forward decl 即可断开循环。
struct WmiResult;
struct WmiThreadContext;
struct WmiThreadState;

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

  // 各种 to*() 都会做类型转换：比如 string "1" 也能 toBool()。
  // 失败时如果提供了 ok 指针，会置为 false 并返回 def。
  [[nodiscard]] bool toBool(bool def = false) const;
  int32_t toInt(bool* ok = nullptr, int32_t def = 0) const;
  uint32_t toUInt(bool* ok = nullptr, uint32_t def = 0) const;
  int64_t toInt64(bool* ok = nullptr, int64_t def = 0) const;
  uint64_t toULongLong(bool* ok = nullptr, uint64_t def = 0) const;
  double toDouble(bool* ok = nullptr, double def = 0.0) const;
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

// ExecMethod 的结果。
struct WmiMethodResult {
  bool attempted = false;  // ExecMethod / ExecMethodAsync 已提交给 WMI；失败也可能已产生副作用
  bool invoked = false;    // WMI 层 ExecMethod 是否成功
  bool returnValuePresent = false;
  uint32_t returnValue = 0;  // returnValuePresent=true 时有效
  int32_t hresult = 0;       // ExecMethod 或响应解码失败的 HRESULT；没有更精确
                             // 的 HRESULT 时保持 0，由 error 描述不完整响应。
  WmiRow outParams;          // 标量 out 参数
  // 数组类型的 out 参数：key=参数名，value=每个元素展开为一行
  // （EmbeddedInstance 会塞进 __MOF；结构体对象会展开为字段 map）。
  std::map<std::string, std::vector<WmiRow>> outArrays;
  std::string error;  // 任何 !ok() 结果的可读信息

  // UWF 方法均声明 UInt32 ReturnValue；缺字段属于不完整响应，不能用默认 0
  // 冒充成功。
  [[nodiscard]] bool ok() const { return invoked && returnValuePresent && returnValue == 0; }
};

enum class WmiClassStatus { Present, Missing, Unknown };
enum class WmiPutMode { CreateOnly, UpdateOnly };

class WmiSession {
 public:
  ~WmiSession();
  WmiSession(const WmiSession&) = delete;
  WmiSession& operator=(const WmiSession&) = delete;

  // 确保当前固定 namespace 已连接；失效代理的释放与重建完全由 session 内部
  // 管理，调用方不参与连接生命周期。
  bool ensureConnected(std::string* error = nullptr) const;

  // 执行一条 WQL 查询，返回每一行的非系统属性集合。若 provider 为结果对象
  // 提供了非空 __PATH，则一并返回；投影查询的 __PATH 合法地可能为 NULL。
  std::vector<WmiRow> query(const std::string& wql, std::string* error = nullptr) const;

  // 查询可被后续方法调用精确定位的实例。通过 WBEM_FLAG_ENSURE_LOCATABLE
  // 请求 WMI 补齐定位信息，并把缺失 __PATH 视为不完整结果。
  std::vector<WmiRow> queryInstances(const std::string& wql, std::string* error = nullptr) const;

  // 检查命名空间里是否注册了某个类。Present / Missing / Unknown 三态严格
  // 区分，调用方不得把连接或权限错误当成“存在”。
  [[nodiscard]] WmiClassStatus classStatus(const std::string& className, std::string* error = nullptr) const;

  // 精确读取一个实例或类对象；读取失败及属性枚举不完整均返回 nullopt。
  [[nodiscard]] std::optional<WmiRow> getObject(const std::string& objectPath, std::string* error = nullptr) const;

  // 调用对象方法。objectPath 可以是：
  //   - 类名（单例或 static 方法）
  //   - 实例的 __PATH（推荐；精确定位一行）
  //   - 相对路径 "Class.Key=Val,..."（不带命名空间，如 ensureNextSessionEntry 构造的）
  // inputs 中的 WmiValue 会根据 kind 转成对应 VARIANT 写入 in-params。
  // 失败时 WmiMethodResult.error 在任何 !ok() 情形下都已填好可读信息，调用方
  // 直接取用即可；callMethod 自身已在 wmi 层记录失败日志。
  [[nodiscard]] WmiMethodResult callMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const;
  // 只读方法允许在明确的 RPC/WMI 连接故障后重建连接并重试一次。写方法必须走
  // callMethod()，绝不能自动重放。
  [[nodiscard]] WmiMethodResult callMethodRead(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const;
  // 可取消的只读方法使用 WMI 原生异步调用；stop 请求会在发起调用的同一线程
  // 执行 CancelAsyncCall，不依赖跨线程 CoCancelCall 的时序窗口。
  [[nodiscard]] WmiMethodResult callMethodReadCancelable(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs,
                                                         std::stop_token stopToken) const;

  // 创建和更新必须由调用方明确选择，禁止用 CREATE_OR_UPDATE 模糊并发语义。
  [[nodiscard]] WmiResult putInstance(const std::string& className, const WmiRow& props, WmiPutMode mode) const;

  // 删除一个 WMI 实例（DeleteInstance），objectPath 用 __PATH 或 relative path。
  [[nodiscard]] WmiResult deleteInstance(const std::string& objectPath) const;

 private:
  friend struct WmiThreadContext;
  // 只允许 WmiThreadContext 创建两个固定 namespace session，避免调用方另建
  // 短生命周期连接或在运行期改绑 namespace。
  explicit WmiSession(std::string namespacePath, const WmiThreadState* threadState);
  std::vector<WmiRow> executeQuery(const std::string& wql, bool ensureLocatable, std::string* error) const;
  bool ensureConnectedWithinOperation(std::string* error) const;
  bool reconnectWithinOperation(std::string* error) const;
  void invalidate() const;

  struct Impl;
  std::unique_ptr<Impl> d;
};

// 每个线程各自拥有两个固定 namespace 的长生命周期 session。首次访问会初始化
// 当前线程的 COM apartment；线程退出时先释放 session，再成对 CoUninitialize。
WmiSession& embeddedWmiSession();
WmiSession& cimv2WmiSession();

// 在启动线程显式建立 COM apartment 与进程安全策略。失败不终止 UI，但会作为
// 后续 session 的稳定 readiness 错误返回给快照/UI。
bool initializeWmiRuntime(std::string* error = nullptr);

// 供领域 API 判断一次写失败是否属于“调用结果可能不确定”的连接故障。此类
// 错误不能重放写方法，只能重连后重新读取状态确认。
[[nodiscard]] bool isWmiConnectionFailure(int32_t hresult);

}  // namespace uwf
