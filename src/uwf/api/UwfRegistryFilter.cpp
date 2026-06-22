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
#include <optional>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf::api {

namespace {

// CommitRegistry / CommitRegistryDeletion 共用的调用骨架——两者除方法名外
// 完全相同。结果走 WmiResult；失败分类（Skipped / Failed）由调用方按需用
// commitOutcome() 派生，见 WmiResult.h。
WmiResult invokeCommit(const WmiSession& session, const std::string& path, const char* method, const std::string& registryKey, const std::string& valueName) {
  if (path.empty()) {
    UWF_LOG_E("UWF_RegistryFilter") << "commit rejected: empty __PATH; method=" << method;
    return WmiResult::failed("UWF_RegistryFilter row has empty __PATH; call read() first");
  }
  WmiRow in;
  // 实机 schema 的参数名是 "Registrykey"（小写 k）；WMI 参数名大小写不敏感，
  // 传 "RegistryKey" 同样生效。
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  in.emplace("ValueName", WmiValue::fromString(valueName));
  auto out = WmiResult::fromMethodResult(session.callMethod(path, method, in));
  if (out.ok) UWF_LOG_I("UWF_RegistryFilter") << std::format("{} ok: key={} value={}", method, registryKey, valueName);
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

namespace {

// AddExclusion / RemoveExclusion 共用：单个 RegistryKey 参数 + 简单 ok/fail。
WmiResult invokeExclusion(const WmiSession& session, const api::RegistryFilterRow& row, const char* method, const std::string& registryKey) {
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method, in));
  if (out.ok) UWF_LOG_I("UWF_RegistryFilter") << method << " ok: key=" << registryKey;
  return out;
}

}  // namespace

WmiResult UwfRegistryFilter::addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  return invokeExclusion(m_session, row, "AddExclusion", registryKey);
}

WmiResult UwfRegistryFilter::removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  return invokeExclusion(m_session, row, "RemoveExclusion", registryKey);
}

WmiResult UwfRegistryFilter::setPersistFlags(const api::RegistryFilterRow& row, bool persistDomainSecretKey, bool persistTSCAL) const {
  // UWF_RegistryFilter 的键属性是 CurrentSession；PutInstance(CREATE_OR_UPDATE)
  // 按键定位实例。本类属性只有 CurrentSession + 这两个布尔，全部给出即整实例更新。
  WmiRow props;
  props.emplace("CurrentSession", WmiValue::fromBool(row.currentSession));
  props.emplace("PersistDomainSecretKey", WmiValue::fromBool(persistDomainSecretKey));
  props.emplace("PersistTSCAL", WmiValue::fromBool(persistTSCAL));
  auto out = m_session.putInstance("UWF_RegistryFilter", props);
  if (out.ok) {
    UWF_LOG_I("UWF_RegistryFilter") << std::format("setPersistFlags ok: session={} dsk={} tscal={}", row.currentSession ? "current" : "next",
                                                   persistDomainSecretKey, persistTSCAL);
  }
  return out;
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
    if (error) *error = methodErrorDetail(r, "UWF_RegistryFilter::FindExclusion");
    return std::nullopt;
  }
  return rowutil::getBool(r.outParams, "bFound");
}

std::vector<api::ExcludedRegistryKey> UwfRegistryFilter::getExclusions(const api::RegistryFilterRow& row, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_RegistryFilter row has empty __PATH; call read() first";
    return {};
  }
  const auto r = m_session.callMethod(row.path, "GetExclusions");
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_RegistryFilter::GetExclusions");
    return {};
  }
  auto out = rowutil::readOutArray<api::ExcludedRegistryKey>(r, "ExcludedKeys", [](const WmiRow& item) -> std::optional<api::ExcludedRegistryKey> {
    api::ExcludedRegistryKey e;
    e.registryKey = rowutil::readExcludedKey(item, "RegistryKey");
    if (e.registryKey.empty()) return std::nullopt;
    return e;
  });
  UWF_LOG_D("UWF_RegistryFilter") << std::format("GetExclusions ok: session={} count={}", row.currentSession ? "current" : "next", out.size());
  return out;
}

WmiResult UwfRegistryFilter::commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  return invokeCommit(m_session, row.path, "CommitRegistry", registryKey, valueName);
}

WmiResult UwfRegistryFilter::commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  return invokeCommit(m_session, row.path, "CommitRegistryDeletion", registryKey, valueName);
}

}  // namespace uwf::api
