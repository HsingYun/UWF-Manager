// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfRegistryFilter.h"

#include <format>
#include <utility>

#include "WriteVerification.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"

namespace uwf::app::provider {

namespace {

provider::RegistryFilterRow decodeRegistryFilter(const wmi::WmiRow& source) {
  return {wmi::row::requireString(source, "__PATH", wmi::row::EmptyString::Reject), wmi::row::requireBool(source, "CurrentSession"),
          wmi::row::requireBool(source, "PersistDomainSecretKey"), wmi::row::requireBool(source, "PersistTSCAL")};
}

provider::RegistryFilterRow rereadRegistryFilter(wmi::WmiOperations& session, const provider::RegistryFilterRow& target) {
  auto observed = decodeRegistryFilter(session.getObject(target.path));
  if (observed.currentSession != target.currentSession)
    throw wmi::WmiProtocolError("reread UWF_RegistryFilter", "provider returned a different session instance");
  return observed;
}

void requirePath(const provider::RegistryFilterRow& row, const char* operation) {
  if (row.path.empty()) throw wmi::WmiProtocolError(operation, "UWF_RegistryFilter row has no object path");
}

}  // namespace

std::vector<provider::RegistryFilterRow> UwfRegistryFilter::readAll() const {
  std::vector<provider::RegistryFilterRow> out;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_RegistryFilter");
  bool seenCurrent = false;
  bool seenNext = false;

  out.reserve(rows.size());
  for (const auto& o : rows) {
    wmi::row::dumpRow("UWF_RegistryFilter", o);
    auto r = decodeRegistryFilter(o);
    bool& seen = r.currentSession ? seenCurrent : seenNext;
    if (seen) {
      throw wmi::WmiProtocolError("read UWF_RegistryFilter", std::format("duplicate {} session instances", r.currentSession ? "current" : "next"));
    }
    seen = true;
    out.push_back(std::move(r));
  }
  UWF_LOG_D("uwf") << "registry filter read completed: rows=" << rows.size();
  return out;
}

provider::RegistryFilterRow UwfRegistryFilter::read(const provider::Session session) const {
  const bool current = session == provider::Session::Current;
  for (auto& r : readAll()) {
    if (r.currentSession == current) return r;
  }
  throw wmi::WmiProtocolError("read UWF_RegistryFilter", std::format("{} session instance is missing", current ? "current" : "next"));
}

void UwfRegistryFilter::addExclusion(const provider::RegistryFilterRow& row, const std::string& registryKey) const {
  requirePath(row, "add UWF registry exclusion");
  wmi::WmiRow in;
  in.emplace("RegistryKey", wmi::WmiValue::fromString(registryKey));
  invokeAndConfirm(
      "add UWF registry exclusion", [&] { m_session.invokeMethod(row.path, "AddExclusion", in); }, [&] { return findExclusion(row, registryKey); });
}

void UwfRegistryFilter::removeExclusion(const provider::RegistryFilterRow& row, const std::string& registryKey) const {
  requirePath(row, "remove UWF registry exclusion");
  wmi::WmiRow in;
  in.emplace("RegistryKey", wmi::WmiValue::fromString(registryKey));
  invokeAndConfirm(
      "remove UWF registry exclusion", [&] { m_session.invokeMethod(row.path, "RemoveExclusion", in); }, [&] { return !findExclusion(row, registryKey); });
}

void UwfRegistryFilter::setPersistence(const provider::RegistryFilterRow& row, const provider::RegistryPersistence persistence) const {
  requirePath(row, "set UWF registry persistence");
  // UWF_RegistryFilter 的键属性是 CurrentSession；UPDATE_ONLY 按键定位既有
  // 实例，绝不在读失败或键错误时意外创建。本类只有键和这两个布尔属性，
  // 全部给出即整实例更新。
  wmi::WmiRow props;
  props.emplace("CurrentSession", wmi::WmiValue::fromBool(row.currentSession));
  props.emplace("PersistDomainSecretKey", wmi::WmiValue::fromBool(persistence.domainSecretKey));
  props.emplace("PersistTSCAL", wmi::WmiValue::fromBool(persistence.terminalServicesClientAccessLicense));
  invokeAndConfirm(
      "set UWF registry persistence", [&] { m_session.putInstance("UWF_RegistryFilter", props, wmi::WmiPutMode::UpdateOnly); },
      [&] {
        const auto observed = rereadRegistryFilter(m_session, row);
        return observed.persistDomainSecretKey == persistence.domainSecretKey && observed.persistTSCAL == persistence.terminalServicesClientAccessLicense;
      });
}

bool UwfRegistryFilter::findExclusion(const provider::RegistryFilterRow& row, const std::string& registryKey) const {
  requirePath(row, "find UWF registry exclusion");
  wmi::WmiRow in;
  in.emplace("RegistryKey", wmi::WmiValue::fromString(registryKey));
  const auto output = m_session.callMethodRead(row.path, "FindExclusion", in);
  return wmi::row::requireBool(output.values, "bFound");
}

std::vector<provider::ExcludedRegistryKey> UwfRegistryFilter::getExclusions(const provider::RegistryFilterRow& row) const {
  requirePath(row, "read UWF registry exclusions");
  const auto output = m_session.callMethodRead(row.path, "GetExclusions");
  auto out = wmi::row::readArrayOutput<provider::ExcludedRegistryKey>(
      output, "ExcludedKeys", [](const wmi::WmiRow& item) { return provider::ExcludedRegistryKey{wmi::row::requireEmbeddedString(item, "RegistryKey")}; });
  UWF_LOG_D("uwf") << "registry exclusions read completed: session=" << (row.currentSession ? "current" : "next") << " count=" << out.size();
  return out;
}

void UwfRegistryFilter::commitRegistry(const provider::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  requirePath(row, "commit UWF registry value");
  wmi::WmiRow inputs;
  inputs.emplace("RegistryKey", wmi::WmiValue::fromString(registryKey));
  inputs.emplace("ValueName", wmi::WmiValue::fromString(valueName));
  m_session.invokeMethod(row.path, "CommitRegistry", inputs);
}

void UwfRegistryFilter::commitRegistryDeletion(const provider::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  requirePath(row, "commit UWF registry deletion");
  wmi::WmiRow inputs;
  inputs.emplace("RegistryKey", wmi::WmiValue::fromString(registryKey));
  inputs.emplace("ValueName", wmi::WmiValue::fromString(valueName));
  m_session.invokeMethod(row.path, "CommitRegistryDeletion", inputs);
}

}  // namespace uwf::app::provider
