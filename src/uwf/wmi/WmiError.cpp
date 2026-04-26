#include "WmiError.h"

#include <algorithm>
#include <array>

namespace uwf {

namespace {

struct Entry {
  int32_t code;
  std::string_view name;
  std::string_view description;
};

// 顺序与 wbemcli.h::WBEMSTATUS 一致。
// clang-format off
constexpr std::array kTable = {
    Entry{0,                                                        "S_OK",                                  "Success."},
    Entry{static_cast<int32_t>(WBEM_E_FAILED),                      "WBEM_E_FAILED",                         "Call failed."},
    Entry{static_cast<int32_t>(WBEM_E_NOT_FOUND),                   "WBEM_E_NOT_FOUND",                      "Object cannot be found."},
    Entry{static_cast<int32_t>(WBEM_E_ACCESS_DENIED),               "WBEM_E_ACCESS_DENIED",                  "Current user does not have permission to perform the action."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_FAILURE),            "WBEM_E_PROVIDER_FAILURE",               "Provider has failed at some time other than during initialization."},
    Entry{static_cast<int32_t>(WBEM_E_TYPE_MISMATCH),               "WBEM_E_TYPE_MISMATCH",                  "Type mismatch occurred."},
    Entry{static_cast<int32_t>(WBEM_E_OUT_OF_MEMORY),               "WBEM_E_OUT_OF_MEMORY",                  "Not enough memory for the operation."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_CONTEXT),             "WBEM_E_INVALID_CONTEXT",                "The IWbemContext object is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_PARAMETER),           "WBEM_E_INVALID_PARAMETER",              "One of the parameters to the call is not correct."},
    Entry{static_cast<int32_t>(WBEM_E_NOT_AVAILABLE),               "WBEM_E_NOT_AVAILABLE",                  "Resource, typically a remote server, is not currently available."},
    Entry{static_cast<int32_t>(WBEM_E_CRITICAL_ERROR),              "WBEM_E_CRITICAL_ERROR",                 "Internal, critical, and unexpected error occurred."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_STREAM),              "WBEM_E_INVALID_STREAM",                 "One or more network packets were corrupted during a remote session."},
    Entry{static_cast<int32_t>(WBEM_E_NOT_SUPPORTED),               "WBEM_E_NOT_SUPPORTED",                  "Feature or operation is not supported."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_SUPERCLASS),          "WBEM_E_INVALID_SUPERCLASS",             "Parent class specified is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_NAMESPACE),           "WBEM_E_INVALID_NAMESPACE",              "Namespace specified cannot be found."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_OBJECT),              "WBEM_E_INVALID_OBJECT",                 "Specified instance is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_CLASS),               "WBEM_E_INVALID_CLASS",                  "Specified class is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_NOT_FOUND),          "WBEM_E_PROVIDER_NOT_FOUND",             "Provider referenced in the schema does not have a corresponding registration."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_PROVIDER_REGISTRATION),"WBEM_E_INVALID_PROVIDER_REGISTRATION", "Provider referenced in the schema has an incorrect or incomplete registration."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_LOAD_FAILURE),       "WBEM_E_PROVIDER_LOAD_FAILURE",          "COM cannot locate a provider referenced in the schema."},
    Entry{static_cast<int32_t>(WBEM_E_INITIALIZATION_FAILURE),      "WBEM_E_INITIALIZATION_FAILURE",         "Component, such as a provider, failed to initialize for internal reasons."},
    Entry{static_cast<int32_t>(WBEM_E_TRANSPORT_FAILURE),           "WBEM_E_TRANSPORT_FAILURE",              "Networking error that prevents normal operation has occurred."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_OPERATION),           "WBEM_E_INVALID_OPERATION",              "Requested operation is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_QUERY),               "WBEM_E_INVALID_QUERY",                  "Query was not syntactically valid."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_QUERY_TYPE),          "WBEM_E_INVALID_QUERY_TYPE",             "Requested query language is not supported."},
    Entry{static_cast<int32_t>(WBEM_E_ALREADY_EXISTS),              "WBEM_E_ALREADY_EXISTS",                 "wbemChangeFlagCreateOnly was specified, but the instance already exists."},
    Entry{static_cast<int32_t>(WBEM_E_OVERRIDE_NOT_ALLOWED),        "WBEM_E_OVERRIDE_NOT_ALLOWED",           "Owning object does not permit overrides."},
    Entry{static_cast<int32_t>(WBEM_E_PROPAGATED_QUALIFIER),        "WBEM_E_PROPAGATED_QUALIFIER",           "User attempted to delete a qualifier that was inherited from a parent class."},
    Entry{static_cast<int32_t>(WBEM_E_PROPAGATED_PROPERTY),         "WBEM_E_PROPAGATED_PROPERTY",            "User attempted to delete a property that was inherited from a parent class."},
    Entry{static_cast<int32_t>(WBEM_E_UNEXPECTED),                  "WBEM_E_UNEXPECTED",                     "Client made an unexpected and illegal sequence of calls."},
    Entry{static_cast<int32_t>(WBEM_E_ILLEGAL_OPERATION),           "WBEM_E_ILLEGAL_OPERATION",              "User requested an illegal operation."},
    Entry{static_cast<int32_t>(WBEM_E_CANNOT_BE_KEY),               "WBEM_E_CANNOT_BE_KEY",                  "Illegal attempt to specify a key qualifier on a property that cannot be a key."},
    Entry{static_cast<int32_t>(WBEM_E_INCOMPLETE_CLASS),            "WBEM_E_INCOMPLETE_CLASS",               "Current object is not a valid class definition."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_SYNTAX),              "WBEM_E_INVALID_SYNTAX",                 "Query is syntactically not valid."},
    Entry{static_cast<int32_t>(WBEM_E_NONDECORATED_OBJECT),         "WBEM_E_NONDECORATED_OBJECT",            "Reserved for future use."},
    Entry{static_cast<int32_t>(WBEM_E_READ_ONLY),                   "WBEM_E_READ_ONLY",                      "An attempt was made to modify a read-only property."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_NOT_CAPABLE),        "WBEM_E_PROVIDER_NOT_CAPABLE",           "Provider cannot perform the requested operation."},
    Entry{static_cast<int32_t>(WBEM_E_CLASS_HAS_CHILDREN),          "WBEM_E_CLASS_HAS_CHILDREN",             "Attempt was made to make a change that invalidates a subclass."},
    Entry{static_cast<int32_t>(WBEM_E_CLASS_HAS_INSTANCES),         "WBEM_E_CLASS_HAS_INSTANCES",            "Attempt was made to delete or modify a class that has instances."},
    Entry{static_cast<int32_t>(WBEM_E_QUERY_NOT_IMPLEMENTED),       "WBEM_E_QUERY_NOT_IMPLEMENTED",          "Reserved for future use."},
    Entry{static_cast<int32_t>(WBEM_E_ILLEGAL_NULL),                "WBEM_E_ILLEGAL_NULL",                   "Null was specified for a property that must have a value."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_QUALIFIER_TYPE),      "WBEM_E_INVALID_QUALIFIER_TYPE",         "Variant value for a qualifier was provided that is not a legal qualifier type."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_PROPERTY_TYPE),       "WBEM_E_INVALID_PROPERTY_TYPE",          "CIM type specified for a property is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_VALUE_OUT_OF_RANGE),          "WBEM_E_VALUE_OUT_OF_RANGE",             "Request was made with an out-of-range value or it is incompatible with the type."},
    Entry{static_cast<int32_t>(WBEM_E_CANNOT_BE_SINGLETON),         "WBEM_E_CANNOT_BE_SINGLETON",            "Illegal attempt was made to make a class singleton."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_CIM_TYPE),            "WBEM_E_INVALID_CIM_TYPE",               "CIM type specified is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_METHOD),              "WBEM_E_INVALID_METHOD",                 "Requested method is not available."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_METHOD_PARAMETERS),   "WBEM_E_INVALID_METHOD_PARAMETERS",      "Parameters provided for the method are not valid."},
    Entry{static_cast<int32_t>(WBEM_E_SYSTEM_PROPERTY),             "WBEM_E_SYSTEM_PROPERTY",                "There was an attempt to get qualifiers on a system property."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_PROPERTY),            "WBEM_E_INVALID_PROPERTY",               "Property type is not recognized."},
    Entry{static_cast<int32_t>(WBEM_E_CALL_CANCELLED),              "WBEM_E_CALL_CANCELLED",                 "Asynchronous process has been canceled internally or by the user."},
    Entry{static_cast<int32_t>(WBEM_E_SHUTTING_DOWN),               "WBEM_E_SHUTTING_DOWN",                  "User has requested an operation while WMI is in the process of shutting down."},
    Entry{static_cast<int32_t>(WBEM_E_PROPAGATED_METHOD),           "WBEM_E_PROPAGATED_METHOD",              "Attempt was made to reuse an existing method name from a parent class and the signatures do not match."},
    Entry{static_cast<int32_t>(WBEM_E_UNSUPPORTED_PARAMETER),       "WBEM_E_UNSUPPORTED_PARAMETER",          "One or more parameter values are too complex or unsupported; retry with simpler parameters."},
    Entry{static_cast<int32_t>(WBEM_E_MISSING_PARAMETER_ID),        "WBEM_E_MISSING_PARAMETER_ID",           "Parameter was missing from the method call."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_PARAMETER_ID),        "WBEM_E_INVALID_PARAMETER_ID",           "Method parameter has an ID qualifier that is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_NONCONSECUTIVE_PARAMETER_IDS),"WBEM_E_NONCONSECUTIVE_PARAMETER_IDS",   "One or more of the method parameters have ID qualifiers that are out of sequence."},
    Entry{static_cast<int32_t>(WBEM_E_PARAMETER_ID_ON_RETVAL),      "WBEM_E_PARAMETER_ID_ON_RETVAL",         "Return value for a method has an ID qualifier."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_OBJECT_PATH),         "WBEM_E_INVALID_OBJECT_PATH",            "Specified object path was not valid."},
    Entry{static_cast<int32_t>(WBEM_E_OUT_OF_DISK_SPACE),           "WBEM_E_OUT_OF_DISK_SPACE",              "Disk is out of space or the 4 GB limit on WMI repository size is reached."},
    Entry{static_cast<int32_t>(WBEM_E_BUFFER_TOO_SMALL),            "WBEM_E_BUFFER_TOO_SMALL",               "Supplied buffer was too small to hold all of the objects in the enumerator or to read a string property."},
    Entry{static_cast<int32_t>(WBEM_E_UNSUPPORTED_PUT_EXTENSION),   "WBEM_E_UNSUPPORTED_PUT_EXTENSION",      "Provider does not support the requested put operation."},
    Entry{static_cast<int32_t>(WBEM_E_UNKNOWN_OBJECT_TYPE),         "WBEM_E_UNKNOWN_OBJECT_TYPE",            "Object with an incorrect type or version was encountered during marshaling."},
    Entry{static_cast<int32_t>(WBEM_E_UNKNOWN_PACKET_TYPE),         "WBEM_E_UNKNOWN_PACKET_TYPE",            "Packet with an incorrect type or version was encountered during marshaling."},
    Entry{static_cast<int32_t>(WBEM_E_MARSHAL_VERSION_MISMATCH),    "WBEM_E_MARSHAL_VERSION_MISMATCH",       "Packet has an unsupported version."},
    Entry{static_cast<int32_t>(WBEM_E_MARSHAL_INVALID_SIGNATURE),   "WBEM_E_MARSHAL_INVALID_SIGNATURE",      "Packet appears to be corrupt."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_QUALIFIER),           "WBEM_E_INVALID_QUALIFIER",              "Attempt was made to mismatch qualifiers."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_DUPLICATE_PARAMETER), "WBEM_E_INVALID_DUPLICATE_PARAMETER",    "Duplicate parameter was declared in a CIM method."},
    Entry{static_cast<int32_t>(WBEM_E_TOO_MUCH_DATA),               "WBEM_E_TOO_MUCH_DATA",                  "Reserved for future use."},
    Entry{static_cast<int32_t>(WBEM_E_SERVER_TOO_BUSY),             "WBEM_E_SERVER_TOO_BUSY",                "Call to IWbemObjectSink::Indicate has failed."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_FLAVOR),              "WBEM_E_INVALID_FLAVOR",                 "Specified qualifier flavor was not valid."},
    Entry{static_cast<int32_t>(WBEM_E_CIRCULAR_REFERENCE),          "WBEM_E_CIRCULAR_REFERENCE",             "Attempt was made to create a circular reference."},
    Entry{static_cast<int32_t>(WBEM_E_UNSUPPORTED_CLASS_UPDATE),    "WBEM_E_UNSUPPORTED_CLASS_UPDATE",       "Specified class is not supported."},
    Entry{static_cast<int32_t>(WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE),  "WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE",  "Attempt was made to change a key when instances or subclasses are already using the key."},
    Entry{static_cast<int32_t>(WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE),"WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE","Attempt was made to change an index when instances or subclasses are already using the index."},
    Entry{static_cast<int32_t>(WBEM_E_TOO_MANY_PROPERTIES),         "WBEM_E_TOO_MANY_PROPERTIES",            "Attempt was made to create more properties than the current version of the class supports."},
    Entry{static_cast<int32_t>(WBEM_E_UPDATE_TYPE_MISMATCH),        "WBEM_E_UPDATE_TYPE_MISMATCH",           "Property was redefined with a conflicting type in a derived class."},
    Entry{static_cast<int32_t>(WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED), "WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED",    "Attempt was made in a derived class to override a qualifier that cannot be overridden."},
    Entry{static_cast<int32_t>(WBEM_E_UPDATE_PROPAGATED_METHOD),    "WBEM_E_UPDATE_PROPAGATED_METHOD",       "Method was re-declared with a conflicting signature in a derived class."},
    Entry{static_cast<int32_t>(WBEM_E_METHOD_NOT_IMPLEMENTED),      "WBEM_E_METHOD_NOT_IMPLEMENTED",         "Attempt was made to execute a method not marked with [implemented] in any relevant class."},
    Entry{static_cast<int32_t>(WBEM_E_METHOD_DISABLED),             "WBEM_E_METHOD_DISABLED",                "Attempt was made to execute a method marked with [disabled]."},
    Entry{static_cast<int32_t>(WBEM_E_REFRESHER_BUSY),              "WBEM_E_REFRESHER_BUSY",                 "Refresher is busy with another operation."},
    Entry{static_cast<int32_t>(WBEM_E_UNPARSABLE_QUERY),            "WBEM_E_UNPARSABLE_QUERY",               "Filtering query is syntactically not valid."},
    Entry{static_cast<int32_t>(WBEM_E_NOT_EVENT_CLASS),             "WBEM_E_NOT_EVENT_CLASS",                "The FROM clause of a filtering query references a class that is not an event class."},
    Entry{static_cast<int32_t>(WBEM_E_MISSING_GROUP_WITHIN),        "WBEM_E_MISSING_GROUP_WITHIN",           "A GROUP BY clause was used without the corresponding GROUP WITHIN clause."},
    Entry{static_cast<int32_t>(WBEM_E_MISSING_AGGREGATION_LIST),    "WBEM_E_MISSING_AGGREGATION_LIST",       "A GROUP BY clause was used; aggregation on all properties is not supported."},
    Entry{static_cast<int32_t>(WBEM_E_PROPERTY_NOT_AN_OBJECT),      "WBEM_E_PROPERTY_NOT_AN_OBJECT",         "Dot notation was used on a property that is not an embedded object."},
    Entry{static_cast<int32_t>(WBEM_E_AGGREGATING_BY_OBJECT),       "WBEM_E_AGGREGATING_BY_OBJECT",          "A GROUP BY clause references a property that is an embedded object without using dot notation."},
    Entry{static_cast<int32_t>(WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY),"WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY","Event provider registration query did not specify the classes for which events were provided."},
    Entry{static_cast<int32_t>(WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING),"WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING","Request was made to back up or restore the repository while it was in use by WinMgmt.exe."},
    Entry{static_cast<int32_t>(WBEM_E_QUEUE_OVERFLOW),              "WBEM_E_QUEUE_OVERFLOW",                 "Asynchronous delivery queue overflowed from the event consumer being too slow."},
    Entry{static_cast<int32_t>(WBEM_E_PRIVILEGE_NOT_HELD),          "WBEM_E_PRIVILEGE_NOT_HELD",             "Operation failed because the client did not have the necessary security privilege."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_OPERATOR),            "WBEM_E_INVALID_OPERATOR",               "Operator is not valid for this property type."},
    Entry{static_cast<int32_t>(WBEM_E_LOCAL_CREDENTIALS),           "WBEM_E_LOCAL_CREDENTIALS",              "User specified credentials on a local connection; use blank credentials and rely on default security."},
    Entry{static_cast<int32_t>(WBEM_E_CANNOT_BE_ABSTRACT),          "WBEM_E_CANNOT_BE_ABSTRACT",             "Class was made abstract when its parent class is not abstract."},
    Entry{static_cast<int32_t>(WBEM_E_AMENDED_OBJECT),              "WBEM_E_AMENDED_OBJECT",                 "Amended object was written without the WBEM_FLAG_USE_AMENDED_QUALIFIERS flag being specified."},
    Entry{static_cast<int32_t>(WBEM_E_CLIENT_TOO_SLOW),             "WBEM_E_CLIENT_TOO_SLOW",                "Client did not retrieve objects quickly enough from an enumeration."},
    Entry{static_cast<int32_t>(WBEM_E_NULL_SECURITY_DESCRIPTOR),    "WBEM_E_NULL_SECURITY_DESCRIPTOR",       "Null security descriptor was used."},
    Entry{static_cast<int32_t>(WBEM_E_TIMED_OUT),                   "WBEM_E_TIMED_OUT",                      "Operation timed out."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_ASSOCIATION),         "WBEM_E_INVALID_ASSOCIATION",            "Association is not valid."},
    Entry{static_cast<int32_t>(WBEM_E_AMBIGUOUS_OPERATION),         "WBEM_E_AMBIGUOUS_OPERATION",            "Operation was ambiguous."},
    Entry{static_cast<int32_t>(WBEM_E_QUOTA_VIOLATION),             "WBEM_E_QUOTA_VIOLATION",                "WMI is taking up too much memory."},
    Entry{static_cast<int32_t>(WBEM_E_UNSUPPORTED_LOCALE),          "WBEM_E_UNSUPPORTED_LOCALE",             "Locale used in the call is not supported."},
    Entry{static_cast<int32_t>(WBEM_E_HANDLE_OUT_OF_DATE),          "WBEM_E_HANDLE_OUT_OF_DATE",             "Object handle is out-of-date."},
    Entry{static_cast<int32_t>(WBEM_E_CONNECTION_FAILED),           "WBEM_E_CONNECTION_FAILED",              "Connection to the SQL database failed."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_HANDLE_REQUEST),      "WBEM_E_INVALID_HANDLE_REQUEST",         "Handle request was not valid."},
    Entry{static_cast<int32_t>(WBEM_E_PROPERTY_NAME_TOO_WIDE),      "WBEM_E_PROPERTY_NAME_TOO_WIDE",         "Property name contains more than 255 characters."},
    Entry{static_cast<int32_t>(WBEM_E_CLASS_NAME_TOO_WIDE),         "WBEM_E_CLASS_NAME_TOO_WIDE",            "Class name contains more than 255 characters."},
    Entry{static_cast<int32_t>(WBEM_E_METHOD_NAME_TOO_WIDE),        "WBEM_E_METHOD_NAME_TOO_WIDE",           "Method name contains more than 255 characters."},
    Entry{static_cast<int32_t>(WBEM_E_QUALIFIER_NAME_TOO_WIDE),     "WBEM_E_QUALIFIER_NAME_TOO_WIDE",        "Qualifier name contains more than 255 characters."},
    Entry{static_cast<int32_t>(WBEM_E_RERUN_COMMAND),               "WBEM_E_RERUN_COMMAND",                  "The SQL command must be rerun because there is a deadlock in SQL."},
    Entry{static_cast<int32_t>(WBEM_E_DATABASE_VER_MISMATCH),       "WBEM_E_DATABASE_VER_MISMATCH",          "The database version does not match the version that the repository driver processes."},
    Entry{static_cast<int32_t>(WBEM_E_VETO_DELETE),                 "WBEM_E_VETO_DELETE",                    "Provider does not allow the delete operation."},
    Entry{static_cast<int32_t>(WBEM_E_VETO_PUT),                    "WBEM_E_VETO_PUT",                       "Provider does not allow the put operation."},
    Entry{static_cast<int32_t>(WBEM_E_INVALID_LOCALE),              "WBEM_E_INVALID_LOCALE",                 "Specified locale identifier was not valid for the operation."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_SUSPENDED),          "WBEM_E_PROVIDER_SUSPENDED",             "Provider is suspended."},
    Entry{static_cast<int32_t>(WBEM_E_SYNCHRONIZATION_REQUIRED),    "WBEM_E_SYNCHRONIZATION_REQUIRED",       "Object must be written and retrieved again before the requested operation can succeed."},
    Entry{static_cast<int32_t>(WBEM_E_NO_SCHEMA),                   "WBEM_E_NO_SCHEMA",                      "Operation cannot be completed; no schema is available."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_ALREADY_REGISTERED), "WBEM_E_PROVIDER_ALREADY_REGISTERED",    "Provider cannot be registered because it is already registered."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_NOT_REGISTERED),     "WBEM_E_PROVIDER_NOT_REGISTERED",        "Provider was not registered."},
    Entry{static_cast<int32_t>(WBEM_E_FATAL_TRANSPORT_ERROR),       "WBEM_E_FATAL_TRANSPORT_ERROR",          "A fatal transport error occurred."},
    Entry{static_cast<int32_t>(WBEM_E_ENCRYPTED_CONNECTION_REQUIRED),"WBEM_E_ENCRYPTED_CONNECTION_REQUIRED", "User attempted to set a computer name or domain without an encrypted connection."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_TIMED_OUT),          "WBEM_E_PROVIDER_TIMED_OUT",             "A provider failed to report results within the specified timeout."},
    Entry{static_cast<int32_t>(WBEM_E_NO_KEY),                      "WBEM_E_NO_KEY",                         "User attempted to put an instance with no defined key."},
    Entry{static_cast<int32_t>(WBEM_E_PROVIDER_DISABLED),           "WBEM_E_PROVIDER_DISABLED",              "User attempted to register a provider instance but the COM server for the provider instance was unloaded."},
};
// clang-format on

const Entry* lookup(int32_t hresult) noexcept {
  // 表内 hresult 视为 unsigned 后是单调递增的，但 std::lower_bound 用 int32_t
  // 比较会因为 0x80041xxx 是负数而出错；这里直接线性扫描，128 项无所谓。
  const auto it = std::ranges::find_if(kTable, [hresult](const Entry& e) { return e.code == hresult; });
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

}  // namespace uwf
