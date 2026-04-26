#include "UwfRegistryFilter.h"

#include <format>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf {

namespace {

bool invokeSimple(const WmiSession& session, const std::string& path, const char* method, const WmiRow& inputs, std::string* error) {
  if (path.empty()) {
    if (error) *error = "UWF_RegistryFilter row has empty __PATH; call read() first";
    UWF_LOG_E("UWF_RegistryFilter") << "invoke rejected: empty __PATH; method=" << method;
    return false;
  }
  const auto r = session.callMethod(path, method, inputs);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_RegistryFilter::{} returned {}", method, r.returnValue) : r.error;
    UWF_LOG_E("UWF_RegistryFilter") << std::format("{} failed: invoked={} rv={} err={}", method, r.invoked, r.returnValue, r.error);
    return false;
  }
  return true;
}

}  // namespace

std::vector<api::RegistryFilterRow> UwfRegistryFilter::readAll(std::string* error) const {
  std::vector<api::RegistryFilterRow> out;
  const auto rows = m_session.query("SELECT * FROM UWF_RegistryFilter", error);

  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_RegistryFilter", o);
    api::RegistryFilterRow r;
    r.path = rowutil::getString(o, "__PATH");
    r.currentSession = rowutil::getBool(o, "CurrentSession");
    r.persistDomainSecretKey = rowutil::getBool(o, "PersistDomainSecretKey");
    r.persistTSCAL = rowutil::getBool(o, "PersistTSCAL");
    out.push_back(std::move(r));
  }
  UWF_LOG_I("UWF_RegistryFilter") << std::format("readAll ok: rows={}", rows.size());
  return out;
}

std::optional<api::RegistryFilterRow> UwfRegistryFilter::read(bool currentSession, std::string* error) const {
  for (auto& r : readAll(error)) {
    if (r.currentSession == currentSession) return r;
  }
  return std::nullopt;
}

bool UwfRegistryFilter::addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error) const {
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  const bool ok = invokeSimple(m_session, row.path, "AddExclusion", in, error);
  if (ok) UWF_LOG_I("UWF_RegistryFilter") << "AddExclusion ok: key=" << registryKey;
  return ok;
}

bool UwfRegistryFilter::removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error) const {
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  const bool ok = invokeSimple(m_session, row.path, "RemoveExclusion", in, error);
  if (ok) UWF_LOG_I("UWF_RegistryFilter") << "RemoveExclusion ok: key=" << registryKey;
  return ok;
}

std::optional<bool> UwfRegistryFilter::findExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_RegistryFilter row has empty __PATH; call read() first";
    return std::nullopt;
  }
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  const auto r = m_session.callMethod(row.path, "FindExclusion", in);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_RegistryFilter::FindExclusion returned {}", r.returnValue) : r.error;
    return std::nullopt;
  }
  return rowutil::getBool(r.outParams, "bFound");
}

std::vector<api::ExcludedRegistryKey> UwfRegistryFilter::getExclusions(const api::RegistryFilterRow& row, std::string* error) const {
  std::vector<api::ExcludedRegistryKey> out;
  if (row.path.empty()) {
    if (error) *error = "UWF_RegistryFilter row has empty __PATH; call read() first";
    return out;
  }
  const auto r = m_session.callMethod(row.path, "GetExclusions");
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_RegistryFilter::GetExclusions returned {}", r.returnValue) : r.error;
    return out;
  }
  const auto it = r.outArrays.find("ExcludedKeys");
  if (it == r.outArrays.end()) return out;

  out.reserve(it->second.size());
  for (const auto& item : it->second) {
    api::ExcludedRegistryKey e;
    e.registryKey = rowutil::readExcludedKey(item, "RegistryKey");
    if (e.registryKey.empty()) continue;
    out.push_back(std::move(e));
  }
  UWF_LOG_D("UWF_RegistryFilter") << std::format("GetExclusions ok: session={} count={}", row.currentSession ? "current" : "next", out.size());
  return out;
}

bool UwfRegistryFilter::commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName,
                                       std::string* error) const {
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  in.emplace("ValueName", WmiValue::fromString(valueName));
  const bool ok = invokeSimple(m_session, row.path, "CommitRegistry", in, error);
  if (ok) UWF_LOG_I("UWF_RegistryFilter") << std::format("CommitRegistry ok: key={} value={}", registryKey, valueName);
  return ok;
}

bool UwfRegistryFilter::commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName,
                                               std::string* error) const {
  WmiRow in;
  // 官方 schema 在此方法名是 "Registrykey"（小写 k），感觉是文档错误。
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  in.emplace("ValueName", WmiValue::fromString(valueName));
  const bool ok = invokeSimple(m_session, row.path, "CommitRegistryDeletion", in, error);
  if (ok) UWF_LOG_I("UWF_RegistryFilter") << std::format("CommitRegistryDeletion ok: key={} value={}", registryKey, valueName);
  return ok;
}

}  // namespace uwf
