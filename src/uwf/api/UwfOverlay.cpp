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
#include "UwfOverlay.h"

#include <format>
#include <regex>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf::api {

namespace {

// 从 UWF_OverlayFile 的 MOF 文本中读取 FileSize。不同 WMI 文本化路径可能把
// UInt64 写成裸数字或带引号的十进制字符串；两种都是同一字段语义。0 是合法
// 的空文件大小，因此用 optional 区分“值为 0”和“字段无法解析”。
std::optional<uint64_t> parseFileSizeFromMof(const std::string& mof) {
  const std::regex re(R"mof(FileSize\s*=\s*(?:"(\d+)"|(\d+))\s*;)mof");
  std::smatch m;
  if (!std::regex_search(mof, m, re)) return std::nullopt;
  try {
    return std::stoull(m[1].matched ? m[1].str() : m[2].str());
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<uint64_t> requireOverlayFileSize(const WmiRow& row, std::string* error) {
  if (const auto direct = row.find("FileSize"); direct != row.end() && direct->second.isValid()) {
    return rowutil::requireUInt64(row, "FileSize", error);
  }
  const auto mof = rowutil::requireString(row, "__MOF", error, false);
  if (!mof) return std::nullopt;
  const auto size = parseFileSizeFromMof(*mof);
  if (!size && error) *error = "WMI field 'FileSize' is missing or invalid in embedded MOF";
  return size;
}

std::optional<api::OverlayRow> decodeOverlay(const WmiRow& source, std::string* error) {
  const auto path = rowutil::requireString(source, "__PATH", error, false);
  const auto consumption = rowutil::requireUInt(source, "OverlayConsumption", error);
  const auto available = rowutil::requireUInt(source, "AvailableSpace", error);
  const auto critical = rowutil::requireUInt(source, "CriticalOverlayThreshold", error);
  const auto warning = rowutil::requireUInt(source, "WarningOverlayThreshold", error);
  if (!path || !consumption || !available || !critical || !warning) return std::nullopt;
  return api::OverlayRow{*path, *consumption, *available, *critical, *warning};
}

std::optional<api::OverlayRow> rereadOverlay(WmiSession& session, const api::OverlayRow& target, std::string* error) {
  const auto source = session.getObject(target.path, error);
  return source ? decodeOverlay(*source, error) : std::nullopt;
}

}  // namespace

std::optional<api::OverlayRow> UwfOverlay::read(std::string* error) const {
  if (error) error->clear();
  std::string queryError;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Overlay", &queryError);
  if (!queryError.empty()) {
    if (error) *error = std::move(queryError);
    return std::nullopt;
  }
  if (rows.size() != 1) {
    if (error) *error = std::format("UWF_Overlay expected one instance, received {}", rows.size());
    UWF_LOG_W("UWF_Overlay") << "read: singleton cardinality mismatch; rows=" << rows.size();
    return std::nullopt;
  }

  const auto& o = rows.front();
  rowutil::dumpRow("UWF_Overlay", o);
  auto r = decodeOverlay(o, error);
  if (!r) return std::nullopt;
  UWF_LOG_D("UWF_Overlay") << std::format("read ok: consumption={}MB available={}MB warning={}MB critical={}MB", r->overlayConsumption, r->availableSpace,
                                          r->warningOverlayThreshold, r->criticalOverlayThreshold);
  return r;
}

std::vector<api::OverlayFileRow> UwfOverlay::getOverlayFiles(const api::OverlayRow& row, const std::string& volume, std::string* error, int32_t* errorCode,
                                                             const std::stop_token stopToken) const {
  std::vector<api::OverlayFileRow> out;
  if (error) error->clear();
  if (errorCode) *errorCode = 0;
  if (row.path.empty()) {
    if (error) *error = "UWF_Overlay row has empty __PATH; call read() first";
    UWF_LOG_E("UWF_Overlay") << "GetOverlayFiles rejected: empty __PATH; volume=" << volume;
    return out;
  }

  WmiRow inputs;
  inputs.emplace("Volume", WmiValue::fromString(volume));

  const auto r = stopToken.stop_possible() ? m_session.callMethodReadCancelable(row.path, "GetOverlayFiles", inputs, stopToken)
                                           : m_session.callMethodRead(row.path, "GetOverlayFiles", inputs);
  if (!r.ok()) {
    const std::string detail = methodErrorDetail(r, "UWF_Overlay::GetOverlayFiles");
    if (error) *error = detail;
    if (errorCode) *errorCode = r.hresult != 0 ? r.hresult : static_cast<int32_t>(r.returnValue);
    if (stopToken.stop_possible() && r.hresult != static_cast<int32_t>(WBEM_E_CALL_CANCELLED)) {
      UWF_LOG_E("UWF_Overlay") << "GetOverlayFiles failed: volume=" << volume << "; " << detail;
    }
    return out;
  }

  auto decoded = rowutil::readArrayOutput<api::OverlayFileRow>(
      r, "OverlayFiles",
      [error](const WmiRow& item) -> std::optional<api::OverlayFileRow> {
        const auto fileName = rowutil::requireEmbeddedString(item, "FileName", error);
        const auto fileSize = requireOverlayFileSize(item, error);
        if (!fileName || !fileSize) return std::nullopt;
        return api::OverlayFileRow{*fileName, *fileSize};
      },
      error);
  if (!decoded) {
    UWF_LOG_E("UWF_Overlay") << "GetOverlayFiles returned an invalid response: " << (error ? *error : std::string{});
    return out;
  }

  out = std::move(*decoded);
  uint64_t totalBytes = 0;
  for (const auto& item : out) totalBytes += item.fileSize;
  UWF_LOG_I("UWF_Overlay") << std::format("GetOverlayFiles ok: volume={} files={} totalBytes={}", volume, out.size(), totalBytes);
  return out;
}

namespace {

// SetWarningThreshold / SetCriticalThreshold 共享前置校验 + WMI 调用骨架。
WmiResult invokeSetThreshold(WmiSession& session, const api::OverlayRow& row, const char* method, uint32_t sizeMb, const bool warning) {
  if (row.path.empty()) return WmiResult::failed("UWF_Overlay row has empty __PATH; call read() first");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method, inputs));

  std::string readError;
  const auto observed = rereadOverlay(session, row, &readError);
  auto verification = verifyObservedState(
      observed,
      [warning, sizeMb](const api::OverlayRow& state) { return (warning ? state.warningOverlayThreshold : state.criticalOverlayThreshold) == sizeMb; },
      std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), std::format("UWF_Overlay::{}", method));
  if (out.ok) UWF_LOG_I("UWF_Overlay") << method << " confirmed: size=" << sizeMb << "MB";
  return out;
}

}  // namespace

WmiResult UwfOverlay::setWarningThreshold(const api::OverlayRow& row, const uint32_t sizeMb) const {
  return invokeSetThreshold(m_session, row, "SetWarningThreshold", sizeMb, true);
}

WmiResult UwfOverlay::setCriticalThreshold(const api::OverlayRow& row, const uint32_t sizeMb) const {
  return invokeSetThreshold(m_session, row, "SetCriticalThreshold", sizeMb, false);
}

}  // namespace uwf::api
