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
#include "UwfRegistryFilter.h"

#include <format>
#include <utility>

#include "../../util/Log.h"
#include "../wmi/WmiException.h"
#include "../wmi/WmiRowUtil.h"
#include "WriteVerification.h"

namespace uwf::api {

namespace {

api::RegistryFilterRow decodeRegistryFilter(const WmiRow& source) {
  return {rowutil::requireString(source, "__PATH", rowutil::EmptyString::Reject), rowutil::requireBool(source, "CurrentSession"),
          rowutil::requireBool(source, "PersistDomainSecretKey"), rowutil::requireBool(source, "PersistTSCAL")};
}

api::RegistryFilterRow rereadRegistryFilter(WmiSession& session, const api::RegistryFilterRow& target) {
  auto observed = decodeRegistryFilter(session.getObject(target.path));
  if (observed.currentSession != target.currentSession) throw WmiProtocolError("reread UWF_RegistryFilter", "provider returned a different session instance");
  return observed;
}

void requirePath(const api::RegistryFilterRow& row, const char* operation) {
  if (row.path.empty()) throw WmiProtocolError(operation, "UWF_RegistryFilter row has no object path");
}

}  // namespace

std::vector<api::RegistryFilterRow> UwfRegistryFilter::readAll() const {
  std::vector<api::RegistryFilterRow> out;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_RegistryFilter");
  bool seenCurrent = false;
  bool seenNext = false;

  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_RegistryFilter", o);
    auto r = decodeRegistryFilter(o);
    bool& seen = r.currentSession ? seenCurrent : seenNext;
    if (seen) {
      throw WmiProtocolError("read UWF_RegistryFilter", std::format("duplicate {} session instances", r.currentSession ? "current" : "next"));
    }
    seen = true;
    out.push_back(std::move(r));
  }
  UWF_LOG_D("uwf") << "registry filter read completed: rows=" << rows.size();
  return out;
}

api::RegistryFilterRow UwfRegistryFilter::read(const api::Session session) const {
  const bool current = session == api::Session::Current;
  for (auto& r : readAll()) {
    if (r.currentSession == current) return r;
  }
  throw WmiProtocolError("read UWF_RegistryFilter", std::format("{} session instance is missing", current ? "current" : "next"));
}

void UwfRegistryFilter::addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  requirePath(row, "add UWF registry exclusion");
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  invokeAndConfirm("add UWF registry exclusion", [&] { m_session.invokeMethod(row.path, "AddExclusion", in); },
                   [&] { return findExclusion(row, registryKey); });
}

void UwfRegistryFilter::removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  requirePath(row, "remove UWF registry exclusion");
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  invokeAndConfirm("remove UWF registry exclusion", [&] { m_session.invokeMethod(row.path, "RemoveExclusion", in); },
                   [&] { return !findExclusion(row, registryKey); });
}

void UwfRegistryFilter::setPersistence(const api::RegistryFilterRow& row, const api::RegistryPersistence persistence) const {
  requirePath(row, "set UWF registry persistence");
  // UWF_RegistryFilter 的键属性是 CurrentSession；UPDATE_ONLY 按键定位既有
  // 实例，绝不在读失败或键错误时意外创建。本类只有键和这两个布尔属性，
  // 全部给出即整实例更新。
  WmiRow props;
  props.emplace("CurrentSession", WmiValue::fromBool(row.currentSession));
  props.emplace("PersistDomainSecretKey", WmiValue::fromBool(persistence.domainSecretKey));
  props.emplace("PersistTSCAL", WmiValue::fromBool(persistence.terminalServicesClientAccessLicense));
  invokeAndConfirm(
      "set UWF registry persistence", [&] { m_session.putInstance("UWF_RegistryFilter", props, WmiPutMode::UpdateOnly); }, [&] {
        const auto observed = rereadRegistryFilter(m_session, row);
        return observed.persistDomainSecretKey == persistence.domainSecretKey && observed.persistTSCAL == persistence.terminalServicesClientAccessLicense;
      });
}

bool UwfRegistryFilter::findExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  requirePath(row, "find UWF registry exclusion");
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  const auto output = m_session.callMethodRead(row.path, "FindExclusion", in);
  return rowutil::requireBool(output.values, "bFound");
}

std::vector<api::ExcludedRegistryKey> UwfRegistryFilter::getExclusions(const api::RegistryFilterRow& row) const {
  requirePath(row, "read UWF registry exclusions");
  const auto output = m_session.callMethodRead(row.path, "GetExclusions");
  auto out = rowutil::readArrayOutput<api::ExcludedRegistryKey>(output, "ExcludedKeys", [](const WmiRow& item) {
    return api::ExcludedRegistryKey{rowutil::requireEmbeddedString(item, "RegistryKey")};
  });
  UWF_LOG_D("uwf") << "registry exclusions read completed: session=" << (row.currentSession ? "current" : "next") << " count=" << out.size();
  return out;
}

void UwfRegistryFilter::commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  requirePath(row, "commit UWF registry value");
  WmiRow inputs;
  inputs.emplace("RegistryKey", WmiValue::fromString(registryKey));
  inputs.emplace("ValueName", WmiValue::fromString(valueName));
  m_session.invokeMethod(row.path, "CommitRegistry", inputs);
}

void UwfRegistryFilter::commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  requirePath(row, "commit UWF registry deletion");
  WmiRow inputs;
  inputs.emplace("RegistryKey", WmiValue::fromString(registryKey));
  inputs.emplace("ValueName", WmiValue::fromString(valueName));
  m_session.invokeMethod(row.path, "CommitRegistryDeletion", inputs);
}

}  // namespace uwf::api
