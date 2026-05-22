#include "UwfRegistryFilter.h"

#include <format>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf {

namespace {

// CommitRegistry / CommitRegistryDeletion 共用的调用 + 结果归类——两者除方法名
// 外完全相同。失败按 classifyCommitFailure 分 Skipped / Failed（见 CommitResult.h）。
CommitResult invokeCommit(const WmiSession& session, const std::string& path, const char* method, const std::string& registryKey,
                          const std::string& valueName) {
  CommitResult out;
  if (path.empty()) {
    out.outcome = CommitOutcome::Failed;
    out.detail = "UWF_RegistryFilter row has empty __PATH; call read() first";
    UWF_LOG_E("UWF_RegistryFilter") << "commit rejected: empty __PATH; method=" << method;
    return out;
  }
  WmiRow in;
  // 实机 schema 的参数名是 "Registrykey"（小写 k）；WMI 参数名大小写不敏感，
  // 传 "RegistryKey" 同样生效。
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  in.emplace("ValueName", WmiValue::fromString(valueName));
  const auto r = session.callMethod(path, method, in);
  out.hresult = r.hresult;
  out.returnValue = r.returnValue;
  if (r.ok()) {
    out.outcome = CommitOutcome::Ok;
    UWF_LOG_I("UWF_RegistryFilter") << std::format("{} ok: key={} value={}", method, registryKey, valueName);
    return out;
  }
  out.detail = r.error;
  out.outcome = classifyCommitFailure(r);
  return out;
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
  const auto r = m_session.callMethod(row.path, "AddExclusion", in);
  if (!r.ok()) {
    if (error) *error = r.error;
    return false;
  }
  UWF_LOG_I("UWF_RegistryFilter") << "AddExclusion ok: key=" << registryKey;
  return true;
}

bool UwfRegistryFilter::removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error) const {
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  const auto r = m_session.callMethod(row.path, "RemoveExclusion", in);
  if (!r.ok()) {
    if (error) *error = r.error;
    return false;
  }
  UWF_LOG_I("UWF_RegistryFilter") << "RemoveExclusion ok: key=" << registryKey;
  return true;
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

CommitResult UwfRegistryFilter::commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  return invokeCommit(m_session, row.path, "CommitRegistry", registryKey, valueName);
}

CommitResult UwfRegistryFilter::commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  return invokeCommit(m_session, row.path, "CommitRegistryDeletion", registryKey, valueName);
}

}  // namespace uwf
