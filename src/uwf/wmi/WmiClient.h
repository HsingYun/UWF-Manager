#pragma once

// 极简 WMI 客户端封装：不依赖 Qt。
//
// WmiValue     —— 类似 QVariant：把 VARIANT 的几种常用类型抹成统一的
//                 {Bool/Int/UInt/Double/String}。
// WmiRow       —— map<属性名, WmiValue>；代表一行查询结果或一个方法的
//                 标量输出参数集合。
// WmiMethodResult —— ExecMethod 的结果：包含 WMI 层是否成功、方法的
//                 UInt32 ReturnValue、以及 out 参数（标量 outParams
//                 和数组类型 outArrays）。
// WmiSession   —— 持有 IWbemLocator / IWbemServices；提供
//                 query() / callMethod()。

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace uwf {

// 前向声明：putInstance / deleteInstance 返回 WmiResult，但 WmiResult.h 反过来
// 依赖本文件（用 WmiMethodResult / WmiError）。.cpp 实现处会 include WmiResult.h，
// 调用方也会 include 它，所以 header 这里只需要 forward decl 即可断开循环。
struct WmiResult;

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

// WmiRow 只是给 map 加一个 "取不到就返回空值" 的便捷函数。
class WmiRow : public std::map<std::string, WmiValue> {
 public:
  [[nodiscard]] WmiValue value(const std::string& key) const {
    const auto it = find(key);
    return it == end() ? WmiValue{} : it->second;
  }
};

// ExecMethod 的结果。
struct WmiMethodResult {
  bool invoked = false;      // WMI 层 ExecMethod 是否成功
  uint32_t returnValue = 0;  // 方法返回值（UInt32，UWF 约定 0=成功）
  int32_t hresult = 0;       // invoked=false 时 ExecMethod 的 HRESULT
                             // （0x80041001 = WBEM_E_FAILED 常见于文件占用，
                             // 0x80041002 = WBEM_E_NOT_FOUND 等）。
  WmiRow outParams;          // 标量 out 参数
  // 数组类型的 out 参数：key=参数名，value=每个元素展开为一行
  // （EmbeddedInstance 会塞进 __MOF；结构体对象会展开为字段 map）。
  std::map<std::string, std::vector<WmiRow>> outArrays;
  std::string error;  // invoked=false 时的可读信息

  // returnValue == 0 且 invoked == true 时才算"真的成功"。
  [[nodiscard]] bool ok() const { return invoked && returnValue == 0; }
};

// 进程内只需要初始化一次 COM + CoInitializeSecurity。幂等、线程安全。
bool initComOnce(std::string* error = nullptr);

class WmiSession {
 public:
  WmiSession();
  ~WmiSession();
  WmiSession(const WmiSession&) = delete;
  WmiSession& operator=(const WmiSession&) = delete;

  // 连接指定命名空间，比如 "root\\standardcimv2\\embedded"。
  bool connect(const std::string& namespacePath, std::string* error = nullptr) const;
  [[nodiscard]] bool isConnected() const;

  // 执行一条 WQL 查询，返回每一行的属性集合。每行自动包含 "__PATH"。
  std::vector<WmiRow> query(const std::string& wql, std::string* error = nullptr) const;

  // 检查命名空间里是否注册了某个类。GetObject 取的是 CIM 仓库里的类定义、
  // 不经 provider，故与提权无关。返回 false 仅代表 GetObject 明确报
  // WBEM_E_INVALID_CLASS（确认不存在）；成功或因其它原因失败一律返回 true，
  // 避免把"无法确认"误判成"不存在"。
  [[nodiscard]] bool classExists(const std::string& className) const;

  // 调用对象方法。objectPath 可以是：
  //   - 类名（单例或 static 方法）
  //   - 实例的 __PATH（推荐；精确定位一行）
  //   - 相对路径 "Class.Key=Val,..."（不带命名空间，如 ensureNextSessionEntry 构造的）
  // inputs 中的 WmiValue 会根据 kind 转成对应 VARIANT 写入 in-params。
  // 失败时 WmiMethodResult.error 在任何 !ok() 情形下都已填好可读信息，调用方
  // 直接取用即可；callMethod 自身已在 wmi 层记录失败日志。
  [[nodiscard]] WmiMethodResult callMethod(const std::string& objectPath, const std::string& methodName, const WmiRow& inputs = {}) const;

  // 用 props 创建（或更新）一个 WMI 实例。className 是类名（如 "UWF_Volume"），
  // props 必须包含全部 key 字段（如 UWF_Volume 的 CurrentSession / DriveLetter
  // / VolumeName）。WBEM_FLAG_CREATE_OR_UPDATE：不存在则创建，已存在则更新。
  // 返回 WmiResult.ok 表示 PutInstance 成功；caller 之后自己 query 拿 __PATH
  // （不在这里读，因为 SpawnInstance 出来的 inst 在 Put 后 __PATH 仍可能为空）。
  [[nodiscard]] WmiResult putInstance(const std::string& className, const WmiRow& props) const;

  // 删除一个 WMI 实例（DeleteInstance），objectPath 用 __PATH 或 relative path。
  [[nodiscard]] WmiResult deleteInstance(const std::string& objectPath) const;

 private:
  struct Impl;
  std::unique_ptr<Impl> d;
};

}  // namespace uwf
