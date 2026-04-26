#pragma once

// WMI 错误码（HRESULT）的强类型封装。
//
// 数据来源：https://learn.microsoft.com/en-us/windows/win32/wmisdk/wmi-error-constants
// 枚举值直接引用 <wbemcli.h> 的 WBEM_E_* 宏，避免与 SDK 漂移。
//
// 用法：
//   WmiError err(r.hresult);
//   if (err == WmiErrorCode::NotFound) { ... }
//   logger << err.name() << ": " << err.description();

#include <windows.h>
// clang-format off
#include <wbemcli.h>
// clang-format on

#include <compare>
#include <cstdint>
#include <string_view>

namespace uwf {

// HRESULT 形式的 WMI 错误码。值与 wbemcli.h 中的 WBEMSTATUS / WBEM_E_* 一致。
// None = 0 是 S_OK。
// clang-format off
enum class WmiErrorCode : int32_t {
  None                          = 0,
  Failed                        = static_cast<int32_t>(WBEM_E_FAILED),
  NotFound                      = static_cast<int32_t>(WBEM_E_NOT_FOUND),
  AccessDenied                  = static_cast<int32_t>(WBEM_E_ACCESS_DENIED),
  ProviderFailure               = static_cast<int32_t>(WBEM_E_PROVIDER_FAILURE),
  TypeMismatch                  = static_cast<int32_t>(WBEM_E_TYPE_MISMATCH),
  OutOfMemory                   = static_cast<int32_t>(WBEM_E_OUT_OF_MEMORY),
  InvalidContext                = static_cast<int32_t>(WBEM_E_INVALID_CONTEXT),
  InvalidParameter              = static_cast<int32_t>(WBEM_E_INVALID_PARAMETER),
  NotAvailable                  = static_cast<int32_t>(WBEM_E_NOT_AVAILABLE),
  CriticalError                 = static_cast<int32_t>(WBEM_E_CRITICAL_ERROR),
  InvalidStream                 = static_cast<int32_t>(WBEM_E_INVALID_STREAM),
  NotSupported                  = static_cast<int32_t>(WBEM_E_NOT_SUPPORTED),
  InvalidSuperclass             = static_cast<int32_t>(WBEM_E_INVALID_SUPERCLASS),
  InvalidNamespace              = static_cast<int32_t>(WBEM_E_INVALID_NAMESPACE),
  InvalidObject                 = static_cast<int32_t>(WBEM_E_INVALID_OBJECT),
  InvalidClass                  = static_cast<int32_t>(WBEM_E_INVALID_CLASS),
  ProviderNotFound              = static_cast<int32_t>(WBEM_E_PROVIDER_NOT_FOUND),
  InvalidProviderRegistration   = static_cast<int32_t>(WBEM_E_INVALID_PROVIDER_REGISTRATION),
  ProviderLoadFailure           = static_cast<int32_t>(WBEM_E_PROVIDER_LOAD_FAILURE),
  InitializationFailure         = static_cast<int32_t>(WBEM_E_INITIALIZATION_FAILURE),
  TransportFailure              = static_cast<int32_t>(WBEM_E_TRANSPORT_FAILURE),
  InvalidOperation              = static_cast<int32_t>(WBEM_E_INVALID_OPERATION),
  InvalidQuery                  = static_cast<int32_t>(WBEM_E_INVALID_QUERY),
  InvalidQueryType              = static_cast<int32_t>(WBEM_E_INVALID_QUERY_TYPE),
  AlreadyExists                 = static_cast<int32_t>(WBEM_E_ALREADY_EXISTS),
  OverrideNotAllowed            = static_cast<int32_t>(WBEM_E_OVERRIDE_NOT_ALLOWED),
  PropagatedQualifier           = static_cast<int32_t>(WBEM_E_PROPAGATED_QUALIFIER),
  PropagatedProperty            = static_cast<int32_t>(WBEM_E_PROPAGATED_PROPERTY),
  Unexpected                    = static_cast<int32_t>(WBEM_E_UNEXPECTED),
  IllegalOperation              = static_cast<int32_t>(WBEM_E_ILLEGAL_OPERATION),
  CannotBeKey                   = static_cast<int32_t>(WBEM_E_CANNOT_BE_KEY),
  IncompleteClass               = static_cast<int32_t>(WBEM_E_INCOMPLETE_CLASS),
  InvalidSyntax                 = static_cast<int32_t>(WBEM_E_INVALID_SYNTAX),
  NondecoratedObject            = static_cast<int32_t>(WBEM_E_NONDECORATED_OBJECT),
  ReadOnly                      = static_cast<int32_t>(WBEM_E_READ_ONLY),
  ProviderNotCapable            = static_cast<int32_t>(WBEM_E_PROVIDER_NOT_CAPABLE),
  ClassHasChildren              = static_cast<int32_t>(WBEM_E_CLASS_HAS_CHILDREN),
  ClassHasInstances             = static_cast<int32_t>(WBEM_E_CLASS_HAS_INSTANCES),
  QueryNotImplemented           = static_cast<int32_t>(WBEM_E_QUERY_NOT_IMPLEMENTED),
  IllegalNull                   = static_cast<int32_t>(WBEM_E_ILLEGAL_NULL),
  InvalidQualifierType          = static_cast<int32_t>(WBEM_E_INVALID_QUALIFIER_TYPE),
  InvalidPropertyType           = static_cast<int32_t>(WBEM_E_INVALID_PROPERTY_TYPE),
  ValueOutOfRange               = static_cast<int32_t>(WBEM_E_VALUE_OUT_OF_RANGE),
  CannotBeSingleton             = static_cast<int32_t>(WBEM_E_CANNOT_BE_SINGLETON),
  InvalidCimType                = static_cast<int32_t>(WBEM_E_INVALID_CIM_TYPE),
  InvalidMethod                 = static_cast<int32_t>(WBEM_E_INVALID_METHOD),
  InvalidMethodParameters       = static_cast<int32_t>(WBEM_E_INVALID_METHOD_PARAMETERS),
  SystemProperty                = static_cast<int32_t>(WBEM_E_SYSTEM_PROPERTY),
  InvalidProperty               = static_cast<int32_t>(WBEM_E_INVALID_PROPERTY),
  CallCancelled                 = static_cast<int32_t>(WBEM_E_CALL_CANCELLED),
  ShuttingDown                  = static_cast<int32_t>(WBEM_E_SHUTTING_DOWN),
  PropagatedMethod              = static_cast<int32_t>(WBEM_E_PROPAGATED_METHOD),
  UnsupportedParameter          = static_cast<int32_t>(WBEM_E_UNSUPPORTED_PARAMETER),
  MissingParameterId            = static_cast<int32_t>(WBEM_E_MISSING_PARAMETER_ID),
  InvalidParameterId            = static_cast<int32_t>(WBEM_E_INVALID_PARAMETER_ID),
  NonconsecutiveParameterIds    = static_cast<int32_t>(WBEM_E_NONCONSECUTIVE_PARAMETER_IDS),
  ParameterIdOnRetval           = static_cast<int32_t>(WBEM_E_PARAMETER_ID_ON_RETVAL),
  InvalidObjectPath             = static_cast<int32_t>(WBEM_E_INVALID_OBJECT_PATH),
  OutOfDiskSpace                = static_cast<int32_t>(WBEM_E_OUT_OF_DISK_SPACE),
  BufferTooSmall                = static_cast<int32_t>(WBEM_E_BUFFER_TOO_SMALL),
  UnsupportedPutExtension       = static_cast<int32_t>(WBEM_E_UNSUPPORTED_PUT_EXTENSION),
  UnknownObjectType             = static_cast<int32_t>(WBEM_E_UNKNOWN_OBJECT_TYPE),
  UnknownPacketType             = static_cast<int32_t>(WBEM_E_UNKNOWN_PACKET_TYPE),
  MarshalVersionMismatch        = static_cast<int32_t>(WBEM_E_MARSHAL_VERSION_MISMATCH),
  MarshalInvalidSignature       = static_cast<int32_t>(WBEM_E_MARSHAL_INVALID_SIGNATURE),
  InvalidQualifier              = static_cast<int32_t>(WBEM_E_INVALID_QUALIFIER),
  InvalidDuplicateParameter     = static_cast<int32_t>(WBEM_E_INVALID_DUPLICATE_PARAMETER),
  TooMuchData                   = static_cast<int32_t>(WBEM_E_TOO_MUCH_DATA),
  ServerTooBusy                 = static_cast<int32_t>(WBEM_E_SERVER_TOO_BUSY),
  InvalidFlavor                 = static_cast<int32_t>(WBEM_E_INVALID_FLAVOR),
  CircularReference             = static_cast<int32_t>(WBEM_E_CIRCULAR_REFERENCE),
  UnsupportedClassUpdate        = static_cast<int32_t>(WBEM_E_UNSUPPORTED_CLASS_UPDATE),
  CannotChangeKeyInheritance    = static_cast<int32_t>(WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE),
  CannotChangeIndexInheritance  = static_cast<int32_t>(WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE),
  TooManyProperties             = static_cast<int32_t>(WBEM_E_TOO_MANY_PROPERTIES),
  UpdateTypeMismatch            = static_cast<int32_t>(WBEM_E_UPDATE_TYPE_MISMATCH),
  UpdateOverrideNotAllowed      = static_cast<int32_t>(WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED),
  UpdatePropagatedMethod        = static_cast<int32_t>(WBEM_E_UPDATE_PROPAGATED_METHOD),
  MethodNotImplemented          = static_cast<int32_t>(WBEM_E_METHOD_NOT_IMPLEMENTED),
  MethodDisabled                = static_cast<int32_t>(WBEM_E_METHOD_DISABLED),
  RefresherBusy                 = static_cast<int32_t>(WBEM_E_REFRESHER_BUSY),
  UnparsableQuery               = static_cast<int32_t>(WBEM_E_UNPARSABLE_QUERY),
  NotEventClass                 = static_cast<int32_t>(WBEM_E_NOT_EVENT_CLASS),
  MissingGroupWithin            = static_cast<int32_t>(WBEM_E_MISSING_GROUP_WITHIN),
  MissingAggregationList        = static_cast<int32_t>(WBEM_E_MISSING_AGGREGATION_LIST),
  PropertyNotAnObject           = static_cast<int32_t>(WBEM_E_PROPERTY_NOT_AN_OBJECT),
  AggregatingByObject           = static_cast<int32_t>(WBEM_E_AGGREGATING_BY_OBJECT),
  UninterpretableProviderQuery  = static_cast<int32_t>(WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY),
  BackupRestoreWinmgmtRunning   = static_cast<int32_t>(WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING),
  QueueOverflow                 = static_cast<int32_t>(WBEM_E_QUEUE_OVERFLOW),
  PrivilegeNotHeld              = static_cast<int32_t>(WBEM_E_PRIVILEGE_NOT_HELD),
  InvalidOperator               = static_cast<int32_t>(WBEM_E_INVALID_OPERATOR),
  LocalCredentials              = static_cast<int32_t>(WBEM_E_LOCAL_CREDENTIALS),
  CannotBeAbstract              = static_cast<int32_t>(WBEM_E_CANNOT_BE_ABSTRACT),
  AmendedObject                 = static_cast<int32_t>(WBEM_E_AMENDED_OBJECT),
  ClientTooSlow                 = static_cast<int32_t>(WBEM_E_CLIENT_TOO_SLOW),
  NullSecurityDescriptor        = static_cast<int32_t>(WBEM_E_NULL_SECURITY_DESCRIPTOR),
  TimedOut                      = static_cast<int32_t>(WBEM_E_TIMED_OUT),
  InvalidAssociation            = static_cast<int32_t>(WBEM_E_INVALID_ASSOCIATION),
  AmbiguousOperation            = static_cast<int32_t>(WBEM_E_AMBIGUOUS_OPERATION),
  QuotaViolation                = static_cast<int32_t>(WBEM_E_QUOTA_VIOLATION),
  UnsupportedLocale             = static_cast<int32_t>(WBEM_E_UNSUPPORTED_LOCALE),
  HandleOutOfDate               = static_cast<int32_t>(WBEM_E_HANDLE_OUT_OF_DATE),
  ConnectionFailed              = static_cast<int32_t>(WBEM_E_CONNECTION_FAILED),
  InvalidHandleRequest          = static_cast<int32_t>(WBEM_E_INVALID_HANDLE_REQUEST),
  PropertyNameTooWide           = static_cast<int32_t>(WBEM_E_PROPERTY_NAME_TOO_WIDE),
  ClassNameTooWide              = static_cast<int32_t>(WBEM_E_CLASS_NAME_TOO_WIDE),
  MethodNameTooWide             = static_cast<int32_t>(WBEM_E_METHOD_NAME_TOO_WIDE),
  QualifierNameTooWide          = static_cast<int32_t>(WBEM_E_QUALIFIER_NAME_TOO_WIDE),
  RerunCommand                  = static_cast<int32_t>(WBEM_E_RERUN_COMMAND),
  DatabaseVerMismatch           = static_cast<int32_t>(WBEM_E_DATABASE_VER_MISMATCH),
  VetoDelete                    = static_cast<int32_t>(WBEM_E_VETO_DELETE),
  VetoPut                       = static_cast<int32_t>(WBEM_E_VETO_PUT),
  InvalidLocale                 = static_cast<int32_t>(WBEM_E_INVALID_LOCALE),
  ProviderSuspended             = static_cast<int32_t>(WBEM_E_PROVIDER_SUSPENDED),
  SynchronizationRequired       = static_cast<int32_t>(WBEM_E_SYNCHRONIZATION_REQUIRED),
  NoSchema                      = static_cast<int32_t>(WBEM_E_NO_SCHEMA),
  ProviderAlreadyRegistered     = static_cast<int32_t>(WBEM_E_PROVIDER_ALREADY_REGISTERED),
  ProviderNotRegistered         = static_cast<int32_t>(WBEM_E_PROVIDER_NOT_REGISTERED),
  FatalTransportError           = static_cast<int32_t>(WBEM_E_FATAL_TRANSPORT_ERROR),
  EncryptedConnectionRequired   = static_cast<int32_t>(WBEM_E_ENCRYPTED_CONNECTION_REQUIRED),
  ProviderTimedOut              = static_cast<int32_t>(WBEM_E_PROVIDER_TIMED_OUT),
  NoKey                         = static_cast<int32_t>(WBEM_E_NO_KEY),
  ProviderDisabled              = static_cast<int32_t>(WBEM_E_PROVIDER_DISABLED),
};
// clang-format on

class WmiError {
 public:
  constexpr WmiError() noexcept = default;
  constexpr explicit WmiError(const int32_t hresult) noexcept : m_hr(hresult) {}
  constexpr explicit WmiError(WmiErrorCode code) noexcept : m_hr(static_cast<int32_t>(code)) {}

  [[nodiscard]] constexpr int32_t hresult() const noexcept { return m_hr; }
  [[nodiscard]] constexpr WmiErrorCode code() const noexcept { return static_cast<WmiErrorCode>(m_hr); }

  // 该 hresult 是否能在 WBEM_E_* 表里查到。
  [[nodiscard]] bool isKnown() const noexcept;
  // hresult == 0（S_OK） 视为无错。
  [[nodiscard]] constexpr bool isFailure() const noexcept { return m_hr != 0; }

  // 助记名（"WBEM_E_FAILED"）。表外返回 "WBEM_E_UNKNOWN"；S_OK 返回 "S_OK"。
  [[nodiscard]] std::string_view name() const noexcept;
  // 微软文档原文（英文）。表外返回空串。
  [[nodiscard]] std::string_view description() const noexcept;

  constexpr bool operator==(WmiErrorCode c) const noexcept { return code() == c; }
  constexpr auto operator<=>(const WmiError&) const noexcept = default;

 private:
  int32_t m_hr = 0;
};

}  // namespace uwf
