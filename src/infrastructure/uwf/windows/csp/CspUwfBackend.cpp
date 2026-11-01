// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/uwf/windows/csp/CspUwfBackend.h"

#include <QUrl>
#include <algorithm>
#include <limits>
#include <set>
#include <type_traits>
#include <utility>

#include "foundation/text/Utf8.h"
#include "infrastructure/uwf/errors/WindowsUwfError.h"
#include "infrastructure/windows/management/csp/WapProvisioningXml.h"

namespace uwf::windows::backends {
namespace {
using namespace compat;
using namespace csp;
constexpr char kBackendName[] = "CSP";
constexpr std::uint32_t kFailure = 0x80004005;
constexpr std::uint32_t kFileNotFound = 0x80070002;
constexpr std::uint32_t kCspNotFound = 0x86000002;
constexpr std::uint32_t kCspNotSupported = 0x86000011;
constexpr std::uint32_t kHresultFailureBit = 0x80000000;
constexpr char kExecUnsupported[] = "The UWF CSP node supports Exec, but this WAP transport has no verified Exec representation";
constexpr char kNextPersistenceUnsupported[] =
    "The captured CSP persistence getter reads Current even at its Next node; next-session observation is unavailable";
constexpr char kTscalReadUnsupported[] =
    "The captured CSP TSCAL getter invokes the native export with an unbalanced critical-section release; this observation is unsafe";

struct PropertyNode {
  Session session;
  QString name;
};

QString text(const std::string& value) { return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size())); }
std::string bytes(const QString& value) { return value.toUtf8().toStdString(); }
QString sessionName(Session session) { return session == Session::Current ? QStringLiteral("CurrentSession") : QStringLiteral("NextSession"); }
QStringList sessionPath(Session session) { return {QStringLiteral("UnifiedWriteFilter"), sessionName(session)}; }

// Encode dynamic names as URI segments before the XML writer escapes attributes.
QString dynamicSegment(const std::string& name) {
  if (name.empty() || name.find('\0') != std::string::npos) throw std::invalid_argument("A dynamic CSP node requires a nonempty name without NUL");
  return QString::fromLatin1(QUrl::toPercentEncoding(text(name)));
}

QString volumeSegment(const std::string& id) { return dynamicSegment(normalizeVolumeId(id)); }
QStringList volumePath(const std::string& id, Session session) {
  auto path = sessionPath(session);
  path << QStringLiteral("Volume") << volumeSegment(id);
  return path;
}

std::optional<PropertyNode> booleanNode(BooleanProperty property, Session session) {
  switch (property) {
    case BooleanProperty::FilterEnabled:
      return PropertyNode{session, QStringLiteral("FilterEnabled")};
    case BooleanProperty::ShutdownPending:
      return PropertyNode{Session::Current, QStringLiteral("ShutdownPending")};
    case BooleanProperty::HormEnabled:
      return PropertyNode{Session::Next, QStringLiteral("HORMEnabled")};
    case BooleanProperty::ServicingEnabled:
      return PropertyNode{session, QStringLiteral("ServicingEnabled")};
    case BooleanProperty::PersistDomainSecretKey:
      if (session == Session::Current) return PropertyNode{session, QStringLiteral("PersistDomainSecretKey")};
      return {};
    case BooleanProperty::PersistTscal:
      return {};
    case BooleanProperty::OverlayCommit:
      return PropertyNode{Session::Current, QStringLiteral("OverlayCommit")};
    case BooleanProperty::RegistryChangesAlwaysPersisted:
    case BooleanProperty::Count:
      return {};
  }
  return {};
}

std::optional<PropertyNode> booleanWriteNode(BooleanProperty property) {
  if (property == BooleanProperty::ShutdownPending) return {};
  if (property == BooleanProperty::PersistDomainSecretKey) return PropertyNode{Session::Next, QStringLiteral("PersistDomainSecretKey")};
  if (property == BooleanProperty::PersistTscal) return PropertyNode{Session::Next, QStringLiteral("PersistTSCAL")};
  return booleanNode(property, property == BooleanProperty::OverlayCommit ? Session::Current : Session::Next);
}

bool isPersistenceProperty(const BooleanProperty property) {
  return property == BooleanProperty::PersistDomainSecretKey || property == BooleanProperty::PersistTscal;
}

std::optional<PropertyNode> numberNode(NumberProperty property, Session session) {
  switch (property) {
    case NumberProperty::OverlayType:
      return PropertyNode{session, QStringLiteral("OverlayType")};
    case NumberProperty::MaximumOverlaySize:
      return PropertyNode{session, QStringLiteral("MaximumOverlaySize")};
    case NumberProperty::OverlayFlags:
      return PropertyNode{session, QStringLiteral("OverlayFlags")};
    case NumberProperty::WarningThreshold:
      return PropertyNode{Session::Current, QStringLiteral("WarningOverlayThreshold")};
    case NumberProperty::CriticalThreshold:
      return PropertyNode{Session::Current, QStringLiteral("CriticalOverlayThreshold")};
    case NumberProperty::ResetPersistentState:
      return PropertyNode{Session::Next, QStringLiteral("ResetPersistentState")};
    case NumberProperty::ResetPersistentStateSavedMode:
      if (session == Session::Next) return PropertyNode{session, QStringLiteral("ResetPersistentStateSavedMode")};
      return {};
    case NumberProperty::Count:
      return {};
  }
  return {};
}

PropertyNode numberWriteNode(NumberProperty property) {
  const auto session = property == NumberProperty::WarningThreshold || property == NumberProperty::CriticalThreshold ? Session::Current : Session::Next;
  const auto node = numberNode(property, session);
  if (!node) throw std::invalid_argument("Unknown UWF numeric property");
  return *node;
}

bool booleanValue(const WapValue& value) {
  if (value.datatype != u"boolean") throw WapProtocolError("Expected a boolean CSP value");
  if (value.text == u"-1" || value.text == u"1" || value.text.compare(u"true", Qt::CaseInsensitive) == 0) return true;
  if (value.text == u"0" || value.text.compare(u"false", Qt::CaseInsensitive) == 0) return false;
  throw WapProtocolError("Invalid CSP boolean value");
}

std::uint32_t numberValue(const WapValue& value, bool signedBits = false) {
  if (value.datatype != u"integer") throw WapProtocolError("Expected an integer CSP value");
  bool valid = !value.text.isEmpty();
  for (qsizetype i = 0; i < value.text.size(); ++i)
    if (!((value.text[i] >= u'0' && value.text[i] <= u'9') || (i == 0 && signedBits && value.text[i] == u'-'))) valid = false;
  bool parsed = false;
  const auto number = value.text.toLongLong(&parsed, 10);
  if (!valid || !parsed || number < (signedBits ? std::numeric_limits<std::int32_t>::min() : 0) || number > std::numeric_limits<std::uint32_t>::max())
    throw WapProtocolError("CSP integer is outside its unsigned 32-bit domain");
  return static_cast<std::uint32_t>(number);
}

std::string stringValue(const WapValue& value) {
  if (!value.datatype.isEmpty() && value.datatype != u"string") throw WapProtocolError("Expected a string CSP value");
  return bytes(value.text);
}

bool exclusionNodeMatches(const QString& name, const std::string& value) {
  if (name == text(value)) return true;  // A literal percent sign is valid stored spelling.
  const auto encoded = name.toUtf8();
  const auto hexadecimal = [](const char character) {
    return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f') || (character >= 'A' && character <= 'F');
  };
  for (qsizetype index = 0; index < encoded.size(); ++index) {
    if (encoded[index] != '%') continue;
    if (index + 2 >= encoded.size() || !hexadecimal(encoded[index + 1]) || !hexadecimal(encoded[index + 2])) return false;
    index += 2;
  }
  const auto decoded = QByteArray::fromPercentEncoding(encoded);
  if (decoded.contains('\0') || !utf8ToUtf16(decoded.toStdString())) return false;
  return decoded == QByteArray::fromStdString(value);
}

ErrorKind errorKind(const std::uint32_t code, const bool writing, const ErrorKind unknown = ErrorKind::Failed) {
  if (const auto kind = classifyWindowsUwfError(code, writing ? WindowsUwfErrorContext::Operation : WindowsUwfErrorContext::Observation)) return *kind;
  switch (code) {
    case kCspNotSupported:
      return ErrorKind::Unsupported;
    case kCspNotFound:
      return ErrorKind::NotFound;
    default:
      return unknown;
  }
}

std::optional<std::string> unsupported(const Request& request) {
  return std::visit(
      [](const auto& operation) -> std::optional<std::string> {
        using R = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<R, GetBoolean>) {
          if (operation.property == BooleanProperty::PersistTscal) return kTscalReadUnsupported;
          if (operation.session == Session::Next && isPersistenceProperty(operation.property)) return kNextPersistenceUnsupported;
          if (!booleanNode(operation.property, operation.session)) return "The UWF CSP has no boolean node for this session";
          return {};
        } else if constexpr (std::is_same_v<R, SetBoolean>) {
          if (!booleanWriteNode(operation.property)) return "The UWF CSP boolean property is read-only";
          return {};
        } else if constexpr (std::is_same_v<R, GetNumber>) {
          if (!numberNode(operation.property, operation.session)) return "The UWF CSP has no numeric node for this session";
          return {};
        } else if constexpr (std::is_same_v<R, FindFileExclusion> || std::is_same_v<R, FindRegistryExclusion>) {
          return "Exclusion lookup is derived from this backend's scoped list";
        } else if constexpr (std::is_same_v<R, SetOverlayOption>) {
          return "Overlay option composition belongs to the UWF abstraction";
        } else if constexpr (std::is_same_v<R, EditFileExclusion> || std::is_same_v<R, EditRegistryExclusion>) {
          if (operation.action == ExclusionAction::Clear) return "Exclusion clearing is composed by the shared configuration command";
          return {};
        } else if constexpr (std::is_same_v<R, GetOverlayFiles> || std::is_same_v<R, UpdateWindows>) {
          return "The UWF CSP does not expose this operation";
        } else if constexpr (std::is_same_v<R, CommitFile> || std::is_same_v<R, CommitFileDeletion> || std::is_same_v<R, CommitRegistry> ||
                             std::is_same_v<R, CommitRegistryDeletion> || std::is_same_v<R, ResetSettings> || std::is_same_v<R, Power>) {
          return kExecUnsupported;
        } else {
          return {};
        }
      },
      request);
}

class Transaction {
 public:
  Transaction(DmCspTransport& transport, const Request& request, const Context& context)
      : m_transport(transport), m_operation(operationName(request)), m_context(context) {}

  [[nodiscard]] Effect writeEffect() const { return m_writeEffect; }
  [[nodiscard]] Diagnostic protocolDiagnostic(const char* message) const {
    return {kBackendName, m_operation, 0, std::string(message) + "\n" + bytes(m_lastResponse)};
  }

  WapDocument perform(const WapCommand& command, bool writing = false) {
    checkCanceled();
    const auto xml = encodeWapCommand(command);
    DmCspReply reply;
    try {
      reply = m_transport.process(xml, {QStringLiteral("UnifiedWriteFilter")});
    } catch (const std::exception& error) {
      if (writing) m_writeEffect = Effect::Uncertain;
      fail(ErrorKind::Failed, "The CSP transport raised an exception", effect(writing), {{kBackendName, m_operation, 0, error.what()}});
    } catch (...) {
      if (writing) m_writeEffect = Effect::Uncertain;
      fail(ErrorKind::Failed, "The CSP transport raised an unknown exception", effect(writing));
    }
    // Record dispatch before response handling can throw and lose write evidence.
    if (writing && reply.invoked) m_writeEffect = Effect::Uncertain;
    m_lastResponse = reply.xml;
    const bool failed = (reply.hresult & kHresultFailureBit) != 0;
    const auto uncertain = effect(writing && reply.invoked);
    std::vector<Diagnostic> diagnostics;
    if (failed || !reply.extendedError.isEmpty()) diagnostics.push_back({kBackendName, m_operation, reply.hresult, bytes(reply.extendedError)});
    std::optional<WapDocument> document;
    if (!reply.xml.isEmpty()) {
      try {
        document = decodeWapDocument(reply.xml);
      } catch (const WapProtocolError& error) {
        diagnostics.push_back({kBackendName, m_operation, 0, std::string(error.what()) + "\n" + bytes(reply.xml)});
        if (!failed) fail(ErrorKind::Protocol, error.what(), uncertain, std::move(diagnostics));
      }
    }
    if (document && !document->errors.empty()) {
      for (const auto& error : document->errors) {
        auto detail = error.element + u" " + error.name + u": " + error.description;
        if (!error.code) detail += u" (provider did not report a valid HRESULT)";
        detail += u"\n" + reply.xml;
        diagnostics.push_back({kBackendName, m_operation, error.code.value_or(0), bytes(detail)});
      }
      const auto code = document->errors.front().code.value_or(failed ? reply.hresult : kFailure);
      fail(errorKind(code, writing), "The UWF CSP returned a provider error", uncertain, std::move(diagnostics));
    }
    // Invocation evidence controls effects, not the meaning of a known error.
    // Only an unclassified pre-dispatch transport status becomes Unavailable.
    if (failed)
      fail(errorKind(reply.hresult, writing, reply.invoked ? ErrorKind::Failed : ErrorKind::Unavailable), "The CSP transport call failed", uncertain,
           std::move(diagnostics));
    if (!reply.invoked) fail(ErrorKind::Unavailable, "The CSP transport did not invoke the provider", effect(false), std::move(diagnostics));
    if (!writing && !document) fail(ErrorKind::Protocol, "The CSP returned no read response", effect(false));
    if (writing) m_writeEffect = Effect::Confirmed;
    return document.value_or(WapDocument{});
  }

  WapValue get(const QStringList& path, const QString& leaf) {
    const auto document = perform({WapAction::GetValue, path, leaf, {}});
    return wapParameter(wapCharacteristic(document, path), leaf);
  }

  void put(const QStringList& path, const QString& leaf, WapValue value) {
    static_cast<void>(perform({WapAction::PutValue, path, leaf, std::move(value)}, true));
  }

  std::vector<std::string> exclusions(const QStringList& path) {
    auto parents = path;
    const auto leaf = parents.takeLast();
    const auto document = perform({WapAction::GetChildren, parents, leaf, {}});
    const auto& container = wapCharacteristic(document, path);
    std::vector<std::string> result;
    std::set<QString> names;
    for (const auto& child : container.children) {
      if (child.element != u"parm" || !child.value) throw WapProtocolError("Expected string children in a CSP exclusion collection");
      if (!names.insert(child.name).second) throw WapProtocolError("Duplicate CSP exclusion node");
      auto entry = stringValue(*child.value);
      // Literal or URI-escaped node names must identify the returned exclusion path.
      if (!exclusionNodeMatches(child.name, entry)) throw WapProtocolError("The CSP exclusion identity does not match its value");
      result.push_back(std::move(entry));
    }
    return result;
  }

  void editExclusion(const QStringList& path, const std::string& name, ExclusionAction action) {
    if (action != ExclusionAction::Add && action != ExclusionAction::Remove) throw std::invalid_argument("Unknown exclusion action");
    const auto child = dynamicSegment(name);
    if (action == ExclusionAction::Add)
      put(path, child, {QStringLiteral("string"), text(name)});
    else
      static_cast<void>(perform({WapAction::RemoveValue, path, child, {}}, true));
  }

 private:
  [[nodiscard]] Effect effect(bool writing) const { return m_writeEffect != Effect::None || writing ? Effect::Uncertain : Effect::None; }
  [[noreturn]] static void fail(ErrorKind kind, std::string message, Effect effect, std::vector<Diagnostic> diagnostics = {}) {
    throw Error({kind, effect, std::move(message), std::move(diagnostics)});
  }
  void checkCanceled() const {
    if (m_context.stop.stop_requested()) fail(ErrorKind::Canceled, "The UWF operation was canceled", effect(false));
  }
  DmCspTransport& m_transport;
  std::string m_operation;
  const Context& m_context;
  Effect m_writeEffect = Effect::None;
  QString m_lastResponse;
};

QStringList fileExclusionPath(const std::string& volume, Session session) {
  auto path = volumePath(volume, session);
  path << QStringLiteral("Exclusions");
  return path;
}
QStringList registryExclusionPath(Session session) {
  auto path = sessionPath(session);
  path << QStringLiteral("RegistryExclusions");
  return path;
}

std::vector<std::pair<std::string, QString>> volumeNames(const WapNode& collection) {
  std::vector<std::pair<std::string, QString>> result;
  std::set<std::string> identifiers;
  for (const auto& child : collection.children) {
    if (child.element != u"characteristic") throw WapProtocolError("Expected volume nodes in a CSP volume collection");
    const auto id = parseVolumeId(bytes(QUrl::fromPercentEncoding(child.name.toUtf8())));
    if (!id) throw WapProtocolError("The CSP volume collection contains an invalid volume identity");
    if (!identifiers.insert(*id).second) throw WapProtocolError("Duplicate CSP volume identifier");
    result.emplace_back(*id, child.name);
  }
  return result;
}

bool volumeProperty(Transaction& transaction, const GetVolumeProperty& request) {
  const auto leaf = request.property == VolumeProperty::Protected ? QStringLiteral("Protected") : QStringLiteral("BindByDriveLetter");
  return booleanValue(transaction.get(volumePath(request.volume, request.session), leaf));
}
std::vector<Volume> volumes(Transaction& transaction, Session session) {
  auto path = sessionPath(session);
  const auto document = transaction.perform({WapAction::GetChildren, path, QStringLiteral("Volume"), {}});
  path << QStringLiteral("Volume");
  std::vector<Volume> result;
  for (const auto& [id, name] : volumeNames(wapCharacteristic(document, path))) {
    auto childPath = path;
    childPath << name;
    std::string driveLetter;
    try {
      driveLetter = stringValue(transaction.get(childPath, QStringLiteral("DriveLetter")));
    } catch (const Error& error) {
      // GUID-only registration may omit DriveLetter; only explicit absence is optional.
      if (error.failure().kind != ErrorKind::NotFound ||
          !std::ranges::any_of(error.failure().diagnostics, [](const auto& diagnostic) { return diagnostic.code == kFileNotFound; }))
        throw;
    }
    result.push_back({id,
                      std::move(driveLetter),
                      volumeProperty(transaction, {id, VolumeProperty::Protected, session}),
                      volumeProperty(transaction, {id, VolumeProperty::BindByDriveLetter, session}),
                      {}});
  }
  return result;
}

template <class T>
Response readSuccess(T value) {
  return Response::success(std::move(value));
}
Response writeSuccess(const Transaction& transaction) { return Response::success(std::monostate{}, transaction.writeEffect()); }

Response executeMapped(Transaction& transaction, const Request& request) {
  return std::visit(
      [&transaction](const auto& operation) -> Response {
        using R = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<R, GetBoolean>) {
          const auto node = *booleanNode(operation.property, operation.session);
          return readSuccess(booleanValue(transaction.get(sessionPath(node.session), node.name)));
        } else if constexpr (std::is_same_v<R, SetBoolean>) {
          const auto node = *booleanWriteNode(operation.property);
          transaction.put(sessionPath(node.session), node.name, {QStringLiteral("boolean"), operation.value ? QStringLiteral("-1") : QStringLiteral("0")});
          return writeSuccess(transaction);
        } else if constexpr (std::is_same_v<R, GetNumber>) {
          const auto node = *numberNode(operation.property, operation.session);
          return readSuccess(numberValue(transaction.get(sessionPath(node.session), node.name), operation.property == NumberProperty::OverlayFlags));
        } else if constexpr (std::is_same_v<R, SetNumber>) {
          const auto node = numberWriteNode(operation.property);
          transaction.put(sessionPath(node.session), node.name, {QStringLiteral("integer"), QString::number(operation.value)});
          return writeSuccess(transaction);
        } else if constexpr (std::is_same_v<R, GetOverlayUsage>) {
          const auto path = sessionPath(Session::Current);
          return readSuccess(OverlayUsage{numberValue(transaction.get(path, QStringLiteral("OverlayConsumption"))),
                                          numberValue(transaction.get(path, QStringLiteral("AvailableOverlaySpace")))});
        } else if constexpr (std::is_same_v<R, GetVolumes>) {
          return readSuccess(volumes(transaction, operation.session));
        } else if constexpr (std::is_same_v<R, AddVolume> || std::is_same_v<R, RemoveVolume>) {
          auto path = sessionPath(Session::Next);
          const auto child = volumeSegment(operation.volume);
          path << QStringLiteral("Volume");
          // CSP derives read-only DriveLetter metadata from the volume identity.
          static_cast<void>(
              transaction.perform({std::is_same_v<R, AddVolume> ? WapAction::AddCharacteristic : WapAction::RemoveCharacteristic, path, child, {}}, true));
          return writeSuccess(transaction);
        } else if constexpr (std::is_same_v<R, GetVolumeProperty>) {
          return readSuccess(volumeProperty(transaction, operation));
        } else if constexpr (std::is_same_v<R, SetVolume>) {
          const auto leaf = operation.property == VolumeProperty::Protected ? QStringLiteral("Protected") : QStringLiteral("BindByDriveLetter");
          const auto path = volumePath(operation.volume, Session::Next);
          transaction.put(path, leaf, {QStringLiteral("boolean"), operation.value ? QStringLiteral("-1") : QStringLiteral("0")});
          return writeSuccess(transaction);
        } else if constexpr (std::is_same_v<R, GetFileExclusions>) {
          return readSuccess(transaction.exclusions(fileExclusionPath(operation.volume, operation.session)));
        } else if constexpr (std::is_same_v<R, EditFileExclusion>) {
          transaction.editExclusion(fileExclusionPath(operation.volume, Session::Next), operation.path, operation.action);
          return writeSuccess(transaction);
        } else if constexpr (std::is_same_v<R, FindFileExclusion> || std::is_same_v<R, FindRegistryExclusion>) {
          throw std::logic_error("Exclusion lookup must use the backend's scoped-list derivation");
        } else if constexpr (std::is_same_v<R, GetRegistryExclusions>) {
          return readSuccess(transaction.exclusions(registryExclusionPath(operation.session)));
        } else if constexpr (std::is_same_v<R, EditRegistryExclusion>) {
          transaction.editExclusion(registryExclusionPath(Session::Next), operation.key, operation.action);
          return writeSuccess(transaction);
        } else if constexpr (std::is_same_v<R, GetSwapfileSize>) {
          return readSuccess(numberValue(transaction.get(volumePath(operation.volume, operation.session), QStringLiteral("SwapfileSize"))));
        } else {
          return Response::fail(ErrorKind::Unsupported, "The request has no supported CSP transport mapping");
        }
      },
      request);
}
template <typename Execute>
Response transact(DmCspTransport& transport, const Request& request, const Context& context, Execute&& execute) {
  Transaction transaction(transport, request, context);
  try {
    return execute(transaction);
  } catch (const Error& error) {
    const auto& failure = error.failure();
    return Response::fail(failure.kind, failure.message, failure.effect, failure.diagnostics);
  } catch (const WapEncodingError& error) {
    // Encoding failed before dispatch, so another backend may try the exact target.
    return Response::fail(ErrorKind::Unsupported, error.what(), transaction.writeEffect());
  } catch (const WapProtocolError& error) {
    return Response::fail(ErrorKind::Protocol, error.what(), transaction.writeEffect(), {transaction.protocolDiagnostic(error.what())});
  } catch (const std::invalid_argument& error) {
    return Response::fail(ErrorKind::InvalidArgument, error.what(), transaction.writeEffect());
  } catch (const std::exception& error) {
    return Response::fail(ErrorKind::Failed, error.what(), transaction.writeEffect(), {{kBackendName, operationName(request), 0, error.what()}});
  } catch (...) {
    return Response::fail(ErrorKind::Failed, "The CSP backend raised an unknown exception", transaction.writeEffect());
  }
}
}  // namespace

CspUwfBackend::CspUwfBackend(std::shared_ptr<DmCspTransport> transport) : m_transport(std::move(transport)) {}

std::string CspUwfBackend::name() const { return kBackendName; }

Capability CspUwfBackend::probeRaw(const Request& request) const {
  if (const auto reason = unsupported(request)) return {Support::Unsupported, *reason};
  try {
    if (!m_transport || !m_transport->available()) return {Support::Unavailable, "The local CSP transport is unavailable"};
  } catch (const std::exception& error) {
    return {Support::Unavailable, error.what()};
  } catch (...) {
    return {Support::Unavailable, "The local CSP transport availability check failed"};
  }
  return {Support::Unknown, "The CSP request is mapped; the provider determines runtime support"};
}

Response CspUwfBackend::executeRaw(const Request& request, const Context& context) const {
  const auto capability = probeRaw(request);
  if (capability.support == Support::Unsupported) return Response::fail(ErrorKind::Unsupported, capability.reason);
  if (capability.support == Support::Unavailable) return Response::fail(ErrorKind::Unavailable, capability.reason);
  return transact(*m_transport, request, context, [&](Transaction& transaction) { return executeMapped(transaction, request); });
}

Response CspUwfBackend::writeSwapfile(const std::string& volume, const std::uint32_t value, const Context& context) const {
  const auto capability = probeRaw(GetSwapfileSize{volume, Session::Next});
  if (capability.support == Support::Unavailable) return Response::fail(ErrorKind::Unavailable, capability.reason);
  return transact(*m_transport, CreateSwapfile{volume}, context, [&](Transaction& transaction) {
    const auto path = volumePath(volume, Session::Next);
    transaction.put(path, QStringLiteral("SwapfileSize"), {QStringLiteral("integer"), QString::number(value)});
    return writeSuccess(transaction);
  });
}

Capability CspUwfBackend::probe(const Request& request) const {
  if (const auto failure = requestInputFailure(request)) return {Support::Unsupported, failure->message};
  return probeRaw(request);
}
Response CspUwfBackend::execute(const Request& request, const Context& context) const {
  if (context.stop.stop_requested()) return Response::fail(ErrorKind::Canceled, "UWF operation canceled before dispatch");
  if (const auto failure = requestInputFailure(request)) return Response::fail(failure->kind, failure->message);
  auto response =
      std::holds_alternative<CreateSwapfile>(request) ? writeSwapfile(std::get<CreateSwapfile>(request).volume, 1, context) : executeRaw(request, context);
  return normalizeResponse(request, std::move(response));
}
}  // namespace uwf::windows::backends
