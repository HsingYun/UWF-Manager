// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Stable WMI and COM transport HRESULT values without Windows SDK dependencies.

#include <compare>
#include <cstdint>
#include <string_view>

namespace uwf::app::wmi {

// clang-format off
enum class WmiErrorCode : int32_t {
  None                          = 0,
  RpcServerFault                = static_cast<int32_t>(0x80010105u),
  Failed                        = static_cast<int32_t>(0x80041001u),
  NotFound                      = static_cast<int32_t>(0x80041002u),
  AccessDenied                  = static_cast<int32_t>(0x80041003u),
  ProviderFailure               = static_cast<int32_t>(0x80041004u),
  TypeMismatch                  = static_cast<int32_t>(0x80041005u),
  OutOfMemory                   = static_cast<int32_t>(0x80041006u),
  InvalidContext                = static_cast<int32_t>(0x80041007u),
  InvalidParameter              = static_cast<int32_t>(0x80041008u),
  NotAvailable                  = static_cast<int32_t>(0x80041009u),
  CriticalError                 = static_cast<int32_t>(0x8004100au),
  InvalidStream                 = static_cast<int32_t>(0x8004100bu),
  NotSupported                  = static_cast<int32_t>(0x8004100cu),
  InvalidSuperclass             = static_cast<int32_t>(0x8004100du),
  InvalidNamespace              = static_cast<int32_t>(0x8004100eu),
  InvalidObject                 = static_cast<int32_t>(0x8004100fu),
  InvalidClass                  = static_cast<int32_t>(0x80041010u),
  ProviderNotFound              = static_cast<int32_t>(0x80041011u),
  InvalidProviderRegistration   = static_cast<int32_t>(0x80041012u),
  ProviderLoadFailure           = static_cast<int32_t>(0x80041013u),
  InitializationFailure         = static_cast<int32_t>(0x80041014u),
  TransportFailure              = static_cast<int32_t>(0x80041015u),
  InvalidOperation              = static_cast<int32_t>(0x80041016u),
  InvalidQuery                  = static_cast<int32_t>(0x80041017u),
  InvalidQueryType              = static_cast<int32_t>(0x80041018u),
  AlreadyExists                 = static_cast<int32_t>(0x80041019u),
  OverrideNotAllowed            = static_cast<int32_t>(0x8004101au),
  PropagatedQualifier           = static_cast<int32_t>(0x8004101bu),
  PropagatedProperty            = static_cast<int32_t>(0x8004101cu),
  Unexpected                    = static_cast<int32_t>(0x8004101du),
  IllegalOperation              = static_cast<int32_t>(0x8004101eu),
  CannotBeKey                   = static_cast<int32_t>(0x8004101fu),
  IncompleteClass               = static_cast<int32_t>(0x80041020u),
  InvalidSyntax                 = static_cast<int32_t>(0x80041021u),
  NondecoratedObject            = static_cast<int32_t>(0x80041022u),
  ReadOnly                      = static_cast<int32_t>(0x80041023u),
  ProviderNotCapable            = static_cast<int32_t>(0x80041024u),
  ClassHasChildren              = static_cast<int32_t>(0x80041025u),
  ClassHasInstances             = static_cast<int32_t>(0x80041026u),
  QueryNotImplemented           = static_cast<int32_t>(0x80041027u),
  IllegalNull                   = static_cast<int32_t>(0x80041028u),
  InvalidQualifierType          = static_cast<int32_t>(0x80041029u),
  InvalidPropertyType           = static_cast<int32_t>(0x8004102au),
  ValueOutOfRange               = static_cast<int32_t>(0x8004102bu),
  CannotBeSingleton             = static_cast<int32_t>(0x8004102cu),
  InvalidCimType                = static_cast<int32_t>(0x8004102du),
  InvalidMethod                 = static_cast<int32_t>(0x8004102eu),
  InvalidMethodParameters       = static_cast<int32_t>(0x8004102fu),
  SystemProperty                = static_cast<int32_t>(0x80041030u),
  InvalidProperty               = static_cast<int32_t>(0x80041031u),
  CallCancelled                 = static_cast<int32_t>(0x80041032u),
  ShuttingDown                  = static_cast<int32_t>(0x80041033u),
  PropagatedMethod              = static_cast<int32_t>(0x80041034u),
  UnsupportedParameter          = static_cast<int32_t>(0x80041035u),
  MissingParameterId            = static_cast<int32_t>(0x80041036u),
  InvalidParameterId            = static_cast<int32_t>(0x80041037u),
  NonconsecutiveParameterIds    = static_cast<int32_t>(0x80041038u),
  ParameterIdOnRetval           = static_cast<int32_t>(0x80041039u),
  InvalidObjectPath             = static_cast<int32_t>(0x8004103au),
  OutOfDiskSpace                = static_cast<int32_t>(0x8004103bu),
  BufferTooSmall                = static_cast<int32_t>(0x8004103cu),
  UnsupportedPutExtension       = static_cast<int32_t>(0x8004103du),
  UnknownObjectType             = static_cast<int32_t>(0x8004103eu),
  UnknownPacketType             = static_cast<int32_t>(0x8004103fu),
  MarshalVersionMismatch        = static_cast<int32_t>(0x80041040u),
  MarshalInvalidSignature       = static_cast<int32_t>(0x80041041u),
  InvalidQualifier              = static_cast<int32_t>(0x80041042u),
  InvalidDuplicateParameter     = static_cast<int32_t>(0x80041043u),
  TooMuchData                   = static_cast<int32_t>(0x80041044u),
  ServerTooBusy                 = static_cast<int32_t>(0x80041045u),
  InvalidFlavor                 = static_cast<int32_t>(0x80041046u),
  CircularReference             = static_cast<int32_t>(0x80041047u),
  UnsupportedClassUpdate        = static_cast<int32_t>(0x80041048u),
  CannotChangeKeyInheritance    = static_cast<int32_t>(0x80041049u),
  CannotChangeIndexInheritance  = static_cast<int32_t>(0x80041050u),
  TooManyProperties             = static_cast<int32_t>(0x80041051u),
  UpdateTypeMismatch            = static_cast<int32_t>(0x80041052u),
  UpdateOverrideNotAllowed      = static_cast<int32_t>(0x80041053u),
  UpdatePropagatedMethod        = static_cast<int32_t>(0x80041054u),
  MethodNotImplemented          = static_cast<int32_t>(0x80041055u),
  MethodDisabled                = static_cast<int32_t>(0x80041056u),
  RefresherBusy                 = static_cast<int32_t>(0x80041057u),
  UnparsableQuery               = static_cast<int32_t>(0x80041058u),
  NotEventClass                 = static_cast<int32_t>(0x80041059u),
  MissingGroupWithin            = static_cast<int32_t>(0x8004105au),
  MissingAggregationList        = static_cast<int32_t>(0x8004105bu),
  PropertyNotAnObject           = static_cast<int32_t>(0x8004105cu),
  AggregatingByObject           = static_cast<int32_t>(0x8004105du),
  UninterpretableProviderQuery  = static_cast<int32_t>(0x8004105fu),
  BackupRestoreWinmgmtRunning   = static_cast<int32_t>(0x80041060u),
  QueueOverflow                 = static_cast<int32_t>(0x80041061u),
  PrivilegeNotHeld              = static_cast<int32_t>(0x80041062u),
  InvalidOperator               = static_cast<int32_t>(0x80041063u),
  LocalCredentials              = static_cast<int32_t>(0x80041064u),
  CannotBeAbstract              = static_cast<int32_t>(0x80041065u),
  AmendedObject                 = static_cast<int32_t>(0x80041066u),
  ClientTooSlow                 = static_cast<int32_t>(0x80041067u),
  NullSecurityDescriptor        = static_cast<int32_t>(0x80041068u),
  TimedOut                      = static_cast<int32_t>(0x80041069u),
  InvalidAssociation            = static_cast<int32_t>(0x8004106au),
  AmbiguousOperation            = static_cast<int32_t>(0x8004106bu),
  QuotaViolation                = static_cast<int32_t>(0x8004106cu),
  UnsupportedLocale             = static_cast<int32_t>(0x8004106fu),
  HandleOutOfDate               = static_cast<int32_t>(0x80041070u),
  ConnectionFailed              = static_cast<int32_t>(0x80041071u),
  InvalidHandleRequest          = static_cast<int32_t>(0x80041072u),
  PropertyNameTooWide           = static_cast<int32_t>(0x80041073u),
  ClassNameTooWide              = static_cast<int32_t>(0x80041074u),
  MethodNameTooWide             = static_cast<int32_t>(0x80041075u),
  QualifierNameTooWide          = static_cast<int32_t>(0x80041076u),
  RerunCommand                  = static_cast<int32_t>(0x80041077u),
  DatabaseVerMismatch           = static_cast<int32_t>(0x80041078u),
  VetoDelete                    = static_cast<int32_t>(0x80041079u),
  VetoPut                       = static_cast<int32_t>(0x8004107au),
  InvalidLocale                 = static_cast<int32_t>(0x80041080u),
  ProviderSuspended             = static_cast<int32_t>(0x80041081u),
  SynchronizationRequired       = static_cast<int32_t>(0x80041082u),
  NoSchema                      = static_cast<int32_t>(0x80041083u),
  ProviderAlreadyRegistered     = static_cast<int32_t>(0x80041084u),
  ProviderNotRegistered         = static_cast<int32_t>(0x80041085u),
  FatalTransportError           = static_cast<int32_t>(0x80041086u),
  EncryptedConnectionRequired   = static_cast<int32_t>(0x80041087u),
  ProviderTimedOut              = static_cast<int32_t>(0x80041088u),
  NoKey                         = static_cast<int32_t>(0x80041089u),
  ProviderDisabled              = static_cast<int32_t>(0x8004108au),
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

}  // namespace uwf::app::wmi
