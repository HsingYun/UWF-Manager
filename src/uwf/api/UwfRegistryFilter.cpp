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

std::optional<api::RegistryFilterRow> decodeRegistryFilter(const WmiRow& source, std::string* error) {
  const auto path = rowutil::requireString(source, "__PATH", error, false);
  const auto currentSession = rowutil::requireBool(source, "CurrentSession", error);
  const auto persistDomainSecretKey = rowutil::requireBool(source, "PersistDomainSecretKey", error);
  const auto persistTSCAL = rowutil::requireBool(source, "PersistTSCAL", error);
  if (!path || !currentSession || !persistDomainSecretKey || !persistTSCAL) return std::nullopt;
  return api::RegistryFilterRow{*path, *currentSession, *persistDomainSecretKey, *persistTSCAL};
}

std::optional<api::RegistryFilterRow> rereadRegistryFilter(WmiSession& session, const api::RegistryFilterRow& target, std::string* error) {
  const auto source = session.getObject(target.path, error);
  if (!source) return std::nullopt;
  auto observed = decodeRegistryFilter(*source, error);
  if (observed && observed->currentSession != target.currentSession) {
    if (error) *error = "UWF_RegistryFilter reread returned a different session instance";
    return std::nullopt;
  }
  return observed;
}

}  // namespace

std::vector<api::RegistryFilterRow> UwfRegistryFilter::readAll(std::string* error) const {
  if (error) error->clear();
  std::vector<api::RegistryFilterRow> out;
  std::string queryError;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_RegistryFilter", &queryError);
  if (!queryError.empty()) {
    if (error) *error = std::move(queryError);
    return out;
  }
  bool seenCurrent = false;
  bool seenNext = false;

  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_RegistryFilter", o);
    auto r = decodeRegistryFilter(o, error);
    if (!r) return {};
    bool& seen = r->currentSession ? seenCurrent : seenNext;
    if (seen) {
      if (error) *error = std::format("UWF_RegistryFilter returned duplicate {} session instances", r->currentSession ? "current" : "next");
      return {};
    }
    seen = true;
    out.push_back(std::move(*r));
  }
  UWF_LOG_I("UWF_RegistryFilter") << std::format("readAll ok: rows={}", rows.size());
  return out;
}

std::optional<api::RegistryFilterRow> UwfRegistryFilter::read(bool currentSession, std::string* error) const {
  for (auto& r : readAll(error)) {
    if (r.currentSession == currentSession) return r;
  }
  if (error && error->empty()) *error = std::format("UWF_RegistryFilter {} session instance is missing", currentSession ? "current" : "next");
  return std::nullopt;
}

namespace {

// AddExclusion / RemoveExclusion 共用：单个 RegistryKey 参数 + 简单 ok/fail。
WmiResult invokeExclusion(WmiSession& session, const api::RegistryFilterRow& row, const char* method, const std::string& registryKey,
                          const bool expectedFound) {
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method, in));
  std::string readError;
  const auto observed = UwfRegistryFilter(session).findExclusion(row, registryKey, &readError);
  auto verification = verifyObservedState(observed, [expectedFound](const bool found) { return found == expectedFound; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), std::format("UWF_RegistryFilter::{}", method));
  if (out.ok) UWF_LOG_I("UWF_RegistryFilter") << method << " confirmed: key=" << registryKey;
  return out;
}

}  // namespace

WmiResult UwfRegistryFilter::addExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  return invokeExclusion(m_session, row, "AddExclusion", registryKey, true);
}

WmiResult UwfRegistryFilter::removeExclusion(const api::RegistryFilterRow& row, const std::string& registryKey) const {
  return invokeExclusion(m_session, row, "RemoveExclusion", registryKey, false);
}

WmiResult UwfRegistryFilter::setPersistFlags(const api::RegistryFilterRow& row, bool persistDomainSecretKey, bool persistTSCAL) const {
  // UWF_RegistryFilter 的键属性是 CurrentSession；UPDATE_ONLY 按键定位既有
  // 实例，绝不在读失败或键错误时意外创建。本类只有键和这两个布尔属性，
  // 全部给出即整实例更新。
  WmiRow props;
  props.emplace("CurrentSession", WmiValue::fromBool(row.currentSession));
  props.emplace("PersistDomainSecretKey", WmiValue::fromBool(persistDomainSecretKey));
  props.emplace("PersistTSCAL", WmiValue::fromBool(persistTSCAL));
  auto out = m_session.putInstance("UWF_RegistryFilter", props, WmiPutMode::UpdateOnly);
  std::string readError;
  const auto observed = rereadRegistryFilter(m_session, row, &readError);
  auto verification = verifyObservedState(
      observed,
      [persistDomainSecretKey, persistTSCAL](const api::RegistryFilterRow& state) {
        return state.persistDomainSecretKey == persistDomainSecretKey && state.persistTSCAL == persistTSCAL;
      },
      std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_RegistryFilter::SetPersistFlags");
  if (out.ok) {
    UWF_LOG_I("UWF_RegistryFilter") << std::format("setPersistFlags confirmed: session={} dsk={} tscal={}", row.currentSession ? "current" : "next",
                                                   persistDomainSecretKey, persistTSCAL);
  }
  return out;
}

std::optional<bool> UwfRegistryFilter::findExclusion(const api::RegistryFilterRow& row, const std::string& registryKey, std::string* error) const {
  if (error) error->clear();
  if (row.path.empty()) {
    if (error) *error = "UWF_RegistryFilter row has empty __PATH; call read() first";
    return std::nullopt;
  }
  WmiRow in;
  in.emplace("RegistryKey", WmiValue::fromString(registryKey));
  const auto r = m_session.callMethodRead(row.path, "FindExclusion", in);
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_RegistryFilter::FindExclusion");
    return std::nullopt;
  }
  return rowutil::requireBool(r.outParams, "bFound", error);
}

std::optional<std::vector<api::ExcludedRegistryKey>> UwfRegistryFilter::getExclusions(const api::RegistryFilterRow& row, std::string* error) const {
  if (error) error->clear();
  if (row.path.empty()) {
    if (error) *error = "UWF_RegistryFilter row has empty __PATH; call read() first";
    return std::nullopt;
  }
  const auto r = m_session.callMethodRead(row.path, "GetExclusions");
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_RegistryFilter::GetExclusions");
    return std::nullopt;
  }
  auto out = rowutil::readArrayOutput<api::ExcludedRegistryKey>(
      r, "ExcludedKeys",
      [error](const WmiRow& item) -> std::optional<api::ExcludedRegistryKey> {
        api::ExcludedRegistryKey e;
        const auto registryKey = rowutil::requireEmbeddedString(item, "RegistryKey", error);
        if (!registryKey) return std::nullopt;
        e.registryKey = *registryKey;
        return e;
      },
      error);
  if (!out) return std::nullopt;
  UWF_LOG_D("UWF_RegistryFilter") << std::format("GetExclusions ok: session={} count={}", row.currentSession ? "current" : "next", out->size());
  return out;
}

WmiResult UwfRegistryFilter::commitRegistry(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  return invokeCommit(m_session, row.path, "CommitRegistry", registryKey, valueName);
}

WmiResult UwfRegistryFilter::commitRegistryDeletion(const api::RegistryFilterRow& row, const std::string& registryKey, const std::string& valueName) const {
  return invokeCommit(m_session, row.path, "CommitRegistryDeletion", registryKey, valueName);
}

}  // namespace uwf::api
