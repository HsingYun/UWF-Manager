// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "WmiError.h"

#include <algorithm>
#include <array>

namespace uwf::app::wmi {

namespace {

struct Entry {
  WmiErrorCode code;
  std::string_view name;
  std::string_view description;
};

// 顺序与 wbemcli.h::WBEMSTATUS 一致。
// clang-format off
constexpr std::array kTable = {
    Entry{WmiErrorCode::None,                       "S_OK",                                 "Success."},
    Entry{WmiErrorCode::Failed,                     "WBEM_E_FAILED",                        "Call failed."},
    Entry{WmiErrorCode::NotFound,                   "WBEM_E_NOT_FOUND",                     "Object cannot be found."},
    Entry{WmiErrorCode::AccessDenied,               "WBEM_E_ACCESS_DENIED",                 "Current user does not have permission to perform the action."},
    Entry{WmiErrorCode::ProviderFailure,            "WBEM_E_PROVIDER_FAILURE",              "Provider has failed at some time other than during initialization."},
    Entry{WmiErrorCode::TypeMismatch,               "WBEM_E_TYPE_MISMATCH",                 "Type mismatch occurred."},
    Entry{WmiErrorCode::OutOfMemory,                "WBEM_E_OUT_OF_MEMORY",                 "Not enough memory for the operation."},
    Entry{WmiErrorCode::InvalidContext,             "WBEM_E_INVALID_CONTEXT",               "The IWbemContext object is not valid."},
    Entry{WmiErrorCode::InvalidParameter,           "WBEM_E_INVALID_PARAMETER",             "One of the parameters to the call is not correct."},
    Entry{WmiErrorCode::NotAvailable,               "WBEM_E_NOT_AVAILABLE",                 "Resource, typically a remote server, is not currently available."},
    Entry{WmiErrorCode::CriticalError,              "WBEM_E_CRITICAL_ERROR",                "Internal, critical, and unexpected error occurred."},
    Entry{WmiErrorCode::InvalidStream,              "WBEM_E_INVALID_STREAM",                "One or more network packets were corrupted during a remote session."},
    Entry{WmiErrorCode::NotSupported,               "WBEM_E_NOT_SUPPORTED",                 "Feature or operation is not supported."},
    Entry{WmiErrorCode::InvalidSuperclass,          "WBEM_E_INVALID_SUPERCLASS",            "Parent class specified is not valid."},
    Entry{WmiErrorCode::InvalidNamespace,           "WBEM_E_INVALID_NAMESPACE",             "Namespace specified cannot be found."},
    Entry{WmiErrorCode::InvalidObject,              "WBEM_E_INVALID_OBJECT",                "Specified instance is not valid."},
    Entry{WmiErrorCode::InvalidClass,               "WBEM_E_INVALID_CLASS",                 "Specified class is not valid."},
    Entry{WmiErrorCode::ProviderNotFound,           "WBEM_E_PROVIDER_NOT_FOUND",            "Provider referenced in the schema does not have a corresponding registration."},
    Entry{WmiErrorCode::InvalidProviderRegistration,"WBEM_E_INVALID_PROVIDER_REGISTRATION", "Provider referenced in the schema has an incorrect or incomplete registration."},
    Entry{WmiErrorCode::ProviderLoadFailure,        "WBEM_E_PROVIDER_LOAD_FAILURE",         "COM cannot locate a provider referenced in the schema."},
    Entry{WmiErrorCode::InitializationFailure,      "WBEM_E_INITIALIZATION_FAILURE",        "Component, such as a provider, failed to initialize for internal reasons."},
    Entry{WmiErrorCode::TransportFailure,           "WBEM_E_TRANSPORT_FAILURE",             "Networking error that prevents normal operation has occurred."},
    Entry{WmiErrorCode::InvalidOperation,           "WBEM_E_INVALID_OPERATION",             "Requested operation is not valid."},
    Entry{WmiErrorCode::InvalidQuery,               "WBEM_E_INVALID_QUERY",                 "Query was not syntactically valid."},
    Entry{WmiErrorCode::InvalidQueryType,           "WBEM_E_INVALID_QUERY_TYPE",            "Requested query language is not supported."},
    Entry{WmiErrorCode::AlreadyExists,              "WBEM_E_ALREADY_EXISTS",                "wbemChangeFlagCreateOnly was specified, but the instance already exists."},
    Entry{WmiErrorCode::OverrideNotAllowed,         "WBEM_E_OVERRIDE_NOT_ALLOWED",          "Owning object does not permit overrides."},
    Entry{WmiErrorCode::PropagatedQualifier,        "WBEM_E_PROPAGATED_QUALIFIER",          "User attempted to delete a qualifier that was inherited from a parent class."},
    Entry{WmiErrorCode::PropagatedProperty,         "WBEM_E_PROPAGATED_PROPERTY",           "User attempted to delete a property that was inherited from a parent class."},
    Entry{WmiErrorCode::Unexpected,                 "WBEM_E_UNEXPECTED",                    "Client made an unexpected and illegal sequence of calls."},
    Entry{WmiErrorCode::IllegalOperation,           "WBEM_E_ILLEGAL_OPERATION",             "User requested an illegal operation."},
    Entry{WmiErrorCode::CannotBeKey,                "WBEM_E_CANNOT_BE_KEY",                 "Illegal attempt to specify a key qualifier on a property that cannot be a key."},
    Entry{WmiErrorCode::IncompleteClass,            "WBEM_E_INCOMPLETE_CLASS",              "Current object is not a valid class definition."},
    Entry{WmiErrorCode::InvalidSyntax,              "WBEM_E_INVALID_SYNTAX",                "Query is syntactically not valid."},
    Entry{WmiErrorCode::NondecoratedObject,         "WBEM_E_NONDECORATED_OBJECT",           "Reserved for future use."},
    Entry{WmiErrorCode::ReadOnly,                   "WBEM_E_READ_ONLY",                     "An attempt was made to modify a read-only property."},
    Entry{WmiErrorCode::ProviderNotCapable,         "WBEM_E_PROVIDER_NOT_CAPABLE",          "Provider cannot perform the requested operation."},
    Entry{WmiErrorCode::ClassHasChildren,           "WBEM_E_CLASS_HAS_CHILDREN",            "Attempt was made to make a change that invalidates a subclass."},
    Entry{WmiErrorCode::ClassHasInstances,          "WBEM_E_CLASS_HAS_INSTANCES",           "Attempt was made to delete or modify a class that has instances."},
    Entry{WmiErrorCode::QueryNotImplemented,        "WBEM_E_QUERY_NOT_IMPLEMENTED",         "Reserved for future use."},
    Entry{WmiErrorCode::IllegalNull,                "WBEM_E_ILLEGAL_NULL",                  "Null was specified for a property that must have a value."},
    Entry{WmiErrorCode::InvalidQualifierType,       "WBEM_E_INVALID_QUALIFIER_TYPE",        "Variant value for a qualifier was provided that is not a legal qualifier type."},
    Entry{WmiErrorCode::InvalidPropertyType,        "WBEM_E_INVALID_PROPERTY_TYPE",         "CIM type specified for a property is not valid."},
    Entry{WmiErrorCode::ValueOutOfRange,            "WBEM_E_VALUE_OUT_OF_RANGE",            "Request was made with an out-of-range value or it is incompatible with the type."},
    Entry{WmiErrorCode::CannotBeSingleton,          "WBEM_E_CANNOT_BE_SINGLETON",           "Illegal attempt was made to make a class singleton."},
    Entry{WmiErrorCode::InvalidCimType,             "WBEM_E_INVALID_CIM_TYPE",              "CIM type specified is not valid."},
    Entry{WmiErrorCode::InvalidMethod,              "WBEM_E_INVALID_METHOD",                "Requested method is not available."},
    Entry{WmiErrorCode::InvalidMethodParameters,    "WBEM_E_INVALID_METHOD_PARAMETERS",     "Parameters provided for the method are not valid."},
    Entry{WmiErrorCode::SystemProperty,             "WBEM_E_SYSTEM_PROPERTY",               "There was an attempt to get qualifiers on a system property."},
    Entry{WmiErrorCode::InvalidProperty,            "WBEM_E_INVALID_PROPERTY",              "Property type is not recognized."},
    Entry{WmiErrorCode::CallCancelled,              "WBEM_E_CALL_CANCELLED",                "Asynchronous process has been canceled internally or by the user."},
    Entry{WmiErrorCode::ShuttingDown,               "WBEM_E_SHUTTING_DOWN",                 "User has requested an operation while WMI is in the process of shutting down."},
    Entry{WmiErrorCode::PropagatedMethod,           "WBEM_E_PROPAGATED_METHOD",             "Attempt was made to reuse an existing method name from a parent class and the signatures do not match."},
    Entry{WmiErrorCode::UnsupportedParameter,       "WBEM_E_UNSUPPORTED_PARAMETER",         "One or more parameter values are too complex or unsupported; retry with simpler parameters."},
    Entry{WmiErrorCode::MissingParameterId,         "WBEM_E_MISSING_PARAMETER_ID",          "Parameter was missing from the method call."},
    Entry{WmiErrorCode::InvalidParameterId,         "WBEM_E_INVALID_PARAMETER_ID",          "Method parameter has an ID qualifier that is not valid."},
    Entry{WmiErrorCode::NonconsecutiveParameterIds,"WBEM_E_NONCONSECUTIVE_PARAMETER_IDS",   "One or more of the method parameters have ID qualifiers that are out of sequence."},
    Entry{WmiErrorCode::ParameterIdOnRetval,        "WBEM_E_PARAMETER_ID_ON_RETVAL",        "Return value for a method has an ID qualifier."},
    Entry{WmiErrorCode::InvalidObjectPath,          "WBEM_E_INVALID_OBJECT_PATH",           "Specified object path was not valid."},
    Entry{WmiErrorCode::OutOfDiskSpace,             "WBEM_E_OUT_OF_DISK_SPACE",             "Disk is out of space or the 4 GB limit on WMI repository size is reached."},
    Entry{WmiErrorCode::BufferTooSmall,             "WBEM_E_BUFFER_TOO_SMALL",              "Supplied buffer was too small to hold all of the objects in the enumerator or to read a string property."},
    Entry{WmiErrorCode::UnsupportedPutExtension,    "WBEM_E_UNSUPPORTED_PUT_EXTENSION",     "Provider does not support the requested put operation."},
    Entry{WmiErrorCode::UnknownObjectType,          "WBEM_E_UNKNOWN_OBJECT_TYPE",           "Object with an incorrect type or version was encountered during marshaling."},
    Entry{WmiErrorCode::UnknownPacketType,          "WBEM_E_UNKNOWN_PACKET_TYPE",           "Packet with an incorrect type or version was encountered during marshaling."},
    Entry{WmiErrorCode::MarshalVersionMismatch,     "WBEM_E_MARSHAL_VERSION_MISMATCH",      "Packet has an unsupported version."},
    Entry{WmiErrorCode::MarshalInvalidSignature,    "WBEM_E_MARSHAL_INVALID_SIGNATURE",     "Packet appears to be corrupt."},
    Entry{WmiErrorCode::InvalidQualifier,           "WBEM_E_INVALID_QUALIFIER",             "Attempt was made to mismatch qualifiers."},
    Entry{WmiErrorCode::InvalidDuplicateParameter,  "WBEM_E_INVALID_DUPLICATE_PARAMETER",   "Duplicate parameter was declared in a CIM method."},
    Entry{WmiErrorCode::TooMuchData,                "WBEM_E_TOO_MUCH_DATA",                 "Reserved for future use."},
    Entry{WmiErrorCode::ServerTooBusy,              "WBEM_E_SERVER_TOO_BUSY",               "Call to IWbemObjectSink::Indicate has failed."},
    Entry{WmiErrorCode::InvalidFlavor,              "WBEM_E_INVALID_FLAVOR",                "Specified qualifier flavor was not valid."},
    Entry{WmiErrorCode::CircularReference,          "WBEM_E_CIRCULAR_REFERENCE",            "Attempt was made to create a circular reference."},
    Entry{WmiErrorCode::UnsupportedClassUpdate,     "WBEM_E_UNSUPPORTED_CLASS_UPDATE",      "Specified class is not supported."},
    Entry{WmiErrorCode::CannotChangeKeyInheritance, "WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE", "Attempt was made to change a key when instances or subclasses are already using the key."},
    Entry{WmiErrorCode::CannotChangeIndexInheritance,"WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE","Attempt was made to change an index when instances or subclasses are already using the index."},
    Entry{WmiErrorCode::TooManyProperties,          "WBEM_E_TOO_MANY_PROPERTIES",           "Attempt was made to create more properties than the current version of the class supports."},
    Entry{WmiErrorCode::UpdateTypeMismatch,         "WBEM_E_UPDATE_TYPE_MISMATCH",          "Property was redefined with a conflicting type in a derived class."},
    Entry{WmiErrorCode::UpdateOverrideNotAllowed,   "WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED",   "Attempt was made in a derived class to override a qualifier that cannot be overridden."},
    Entry{WmiErrorCode::UpdatePropagatedMethod,     "WBEM_E_UPDATE_PROPAGATED_METHOD",      "Method was re-declared with a conflicting signature in a derived class."},
    Entry{WmiErrorCode::MethodNotImplemented,       "WBEM_E_METHOD_NOT_IMPLEMENTED",        "Attempt was made to execute a method not marked with [implemented] in any relevant class."},
    Entry{WmiErrorCode::MethodDisabled,             "WBEM_E_METHOD_DISABLED",               "Attempt was made to execute a method marked with [disabled]."},
    Entry{WmiErrorCode::RefresherBusy,              "WBEM_E_REFRESHER_BUSY",                "Refresher is busy with another operation."},
    Entry{WmiErrorCode::UnparsableQuery,            "WBEM_E_UNPARSABLE_QUERY",              "Filtering query is syntactically not valid."},
    Entry{WmiErrorCode::NotEventClass,              "WBEM_E_NOT_EVENT_CLASS",               "The FROM clause of a filtering query references a class that is not an event class."},
    Entry{WmiErrorCode::MissingGroupWithin,         "WBEM_E_MISSING_GROUP_WITHIN",          "A GROUP BY clause was used without the corresponding GROUP WITHIN clause."},
    Entry{WmiErrorCode::MissingAggregationList,     "WBEM_E_MISSING_AGGREGATION_LIST",      "A GROUP BY clause was used; aggregation on all properties is not supported."},
    Entry{WmiErrorCode::PropertyNotAnObject,        "WBEM_E_PROPERTY_NOT_AN_OBJECT",        "Dot notation was used on a property that is not an embedded object."},
    Entry{WmiErrorCode::AggregatingByObject,        "WBEM_E_AGGREGATING_BY_OBJECT",         "A GROUP BY clause references a property that is an embedded object without using dot notation."},
    Entry{WmiErrorCode::UninterpretableProviderQuery,"WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY","Event provider registration query did not specify the classes for which events were provided."},
    Entry{WmiErrorCode::BackupRestoreWinmgmtRunning,"WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING","Request was made to back up or restore the repository while it was in use by WinMgmt.exe."},
    Entry{WmiErrorCode::QueueOverflow,              "WBEM_E_QUEUE_OVERFLOW",                "Asynchronous delivery queue overflowed from the event consumer being too slow."},
    Entry{WmiErrorCode::PrivilegeNotHeld,           "WBEM_E_PRIVILEGE_NOT_HELD",            "Operation failed because the client did not have the necessary security privilege."},
    Entry{WmiErrorCode::InvalidOperator,            "WBEM_E_INVALID_OPERATOR",              "Operator is not valid for this property type."},
    Entry{WmiErrorCode::LocalCredentials,           "WBEM_E_LOCAL_CREDENTIALS",             "User specified credentials on a local connection; use blank credentials and rely on default security."},
    Entry{WmiErrorCode::CannotBeAbstract,           "WBEM_E_CANNOT_BE_ABSTRACT",            "Class was made abstract when its parent class is not abstract."},
    Entry{WmiErrorCode::AmendedObject,              "WBEM_E_AMENDED_OBJECT",                "Amended object was written without the WBEM_FLAG_USE_AMENDED_QUALIFIERS flag being specified."},
    Entry{WmiErrorCode::ClientTooSlow,              "WBEM_E_CLIENT_TOO_SLOW",               "Client did not retrieve objects quickly enough from an enumeration."},
    Entry{WmiErrorCode::NullSecurityDescriptor,     "WBEM_E_NULL_SECURITY_DESCRIPTOR",      "Null security descriptor was used."},
    Entry{WmiErrorCode::TimedOut,                   "WBEM_E_TIMED_OUT",                     "Operation timed out."},
    Entry{WmiErrorCode::InvalidAssociation,         "WBEM_E_INVALID_ASSOCIATION",           "Association is not valid."},
    Entry{WmiErrorCode::AmbiguousOperation,         "WBEM_E_AMBIGUOUS_OPERATION",           "Operation was ambiguous."},
    Entry{WmiErrorCode::QuotaViolation,             "WBEM_E_QUOTA_VIOLATION",               "WMI is taking up too much memory."},
    Entry{WmiErrorCode::UnsupportedLocale,          "WBEM_E_UNSUPPORTED_LOCALE",            "Locale used in the call is not supported."},
    Entry{WmiErrorCode::HandleOutOfDate,            "WBEM_E_HANDLE_OUT_OF_DATE",            "Object handle is out-of-date."},
    Entry{WmiErrorCode::ConnectionFailed,           "WBEM_E_CONNECTION_FAILED",             "Connection to the SQL database failed."},
    Entry{WmiErrorCode::InvalidHandleRequest,       "WBEM_E_INVALID_HANDLE_REQUEST",        "Handle request was not valid."},
    Entry{WmiErrorCode::PropertyNameTooWide,        "WBEM_E_PROPERTY_NAME_TOO_WIDE",        "Property name contains more than 255 characters."},
    Entry{WmiErrorCode::ClassNameTooWide,           "WBEM_E_CLASS_NAME_TOO_WIDE",           "Class name contains more than 255 characters."},
    Entry{WmiErrorCode::MethodNameTooWide,          "WBEM_E_METHOD_NAME_TOO_WIDE",          "Method name contains more than 255 characters."},
    Entry{WmiErrorCode::QualifierNameTooWide,       "WBEM_E_QUALIFIER_NAME_TOO_WIDE",       "Qualifier name contains more than 255 characters."},
    Entry{WmiErrorCode::RerunCommand,               "WBEM_E_RERUN_COMMAND",                 "The SQL command must be rerun because there is a deadlock in SQL."},
    Entry{WmiErrorCode::DatabaseVerMismatch,        "WBEM_E_DATABASE_VER_MISMATCH",         "The database version does not match the version that the repository driver processes."},
    Entry{WmiErrorCode::VetoDelete,                 "WBEM_E_VETO_DELETE",                   "Provider does not allow the delete operation."},
    Entry{WmiErrorCode::VetoPut,                    "WBEM_E_VETO_PUT",                      "Provider does not allow the put operation."},
    Entry{WmiErrorCode::InvalidLocale,              "WBEM_E_INVALID_LOCALE",                "Specified locale identifier was not valid for the operation."},
    Entry{WmiErrorCode::ProviderSuspended,          "WBEM_E_PROVIDER_SUSPENDED",            "Provider is suspended."},
    Entry{WmiErrorCode::SynchronizationRequired,    "WBEM_E_SYNCHRONIZATION_REQUIRED",      "Object must be written and retrieved again before the requested operation can succeed."},
    Entry{WmiErrorCode::NoSchema,                   "WBEM_E_NO_SCHEMA",                     "Operation cannot be completed; no schema is available."},
    Entry{WmiErrorCode::ProviderAlreadyRegistered,  "WBEM_E_PROVIDER_ALREADY_REGISTERED",   "Provider cannot be registered because it is already registered."},
    Entry{WmiErrorCode::ProviderNotRegistered,      "WBEM_E_PROVIDER_NOT_REGISTERED",       "Provider was not registered."},
    Entry{WmiErrorCode::FatalTransportError,        "WBEM_E_FATAL_TRANSPORT_ERROR",         "A fatal transport error occurred."},
    Entry{WmiErrorCode::EncryptedConnectionRequired,"WBEM_E_ENCRYPTED_CONNECTION_REQUIRED", "User attempted to set a computer name or domain without an encrypted connection."},
    Entry{WmiErrorCode::ProviderTimedOut,           "WBEM_E_PROVIDER_TIMED_OUT",            "A provider failed to report results within the specified timeout."},
    Entry{WmiErrorCode::NoKey,                      "WBEM_E_NO_KEY",                        "User attempted to put an instance with no defined key."},
    Entry{WmiErrorCode::ProviderDisabled,           "WBEM_E_PROVIDER_DISABLED",             "User attempted to register a provider instance but the COM server for the provider instance was unloaded."},
};
// clang-format on

const Entry* lookup(int32_t hresult) noexcept {
  // HRESULT 按有符号值比较不符合表的排列顺序；固定大小表使用线性查找。
  const auto code = static_cast<WmiErrorCode>(hresult);
  const auto it = std::ranges::find_if(kTable, [code](const Entry& e) { return e.code == code; });
  return it == kTable.end() ? nullptr : &*it;
}

}  // namespace

bool WmiError::isKnown() const noexcept { return lookup(m_hr) != nullptr; }

std::string_view WmiError::name() const noexcept {
  if (const auto* e = lookup(m_hr)) return e->name;
  return "WBEM_E_UNKNOWN";
}

std::string_view WmiError::description() const noexcept {
  if (const auto* e = lookup(m_hr)) return e->description;
  return {};
}

}  // namespace uwf::app::wmi
