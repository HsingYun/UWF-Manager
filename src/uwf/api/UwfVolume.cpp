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
#include "UwfVolume.h"

#include <format>
#include <optional>

#include "../../util/DriveLetter.h"
#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf::api {

namespace {

// UWF_Volume 的 AddExclusion / RemoveExclusion / FindExclusion / CommitFile /
// CommitFileDeletion 的路径参数均不包含驱动器号或卷名（见
// knowledge/reference/11-uwf-api.html 各方法的参数说明）。若用户传入
// "C:\foo"，这里在确认与目标卷匹配后剥掉开头的 "<letter>:"。盘符不匹配直接
// 返回空 optional 并写 *error。
std::optional<std::string> stripVolumeDriveLetter(const std::string& path, const std::string& volumeDriveLetter, std::string* error) {
  // UWF 的 AddExclusion / CommitFile 等方法收的是"卷内相对路径"（不含盘符）。
  // 这里把用户给的完整路径按盘符拆开：拆分逻辑统一交给 drive::split。
  const auto s = drive::split(path);
  if (s.letter.empty()) return path;  // 无盘符前缀——已是卷内相对路径，原样返回
  if (s.letter != volumeDriveLetter) {
    if (error) *error = std::format("path \"{}\" drive letter does not match target volume {}", path, volumeDriveLetter);
    return std::nullopt;
  }
  return s.rest;
}

// 把字符串转义后嵌入 WMI 对象路径的引号键值（DriveLetter="..." / VolumeName="..."）。
// WMI 对象路径按 C/C++ 规则解析引号转义（见 MS 文档 "WMI Object Path
// Requirements"："embedded quotation marks ... must delimit the quotation mark
// with escape characters, as in a C or C++ application"），故 `"` 要写成 `\"`、
// `\` 要写成 `\\`。
//
// 实测：UWF_Volume.VolumeName 是 "Volume{GUID}" 形式（不带 \\?\ 前缀、不带结尾
// 反斜杠——详见 knowledge/reference/11-uwf-api.html 中 UWF_Volume 的备注），
// DriveLetter 是 "C:" 形式；两者都不含 `\` 或 `"`，所以此处转义当前其实是空操作。
// 保留它作为防御——若将来某个 key 值含特殊字符，合成出的 __PATH 仍 correct-
// by-construction。
// （注：readAll() 里 row.path 取自 __PATH，是 WMI 自己产出、已正确转义的路径，
// 不经过这里；只有本文件 ensureNextSessionEntry 手工拼 __PATH 才用到本函数。）
std::string escapeWmiPathValue(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (const char c : s) {
    if (c == '\\' || c == '"') out += '\\';
    out += c;
  }
  return out;
}

bool decodeManagedVolume(const WmiRow& source, std::optional<api::VolumeRow>& decoded, std::string* error) {
  decoded.reset();
  const auto path = rowutil::requireString(source, "__PATH", error, false);
  const auto currentSession = rowutil::requireBool(source, "CurrentSession", error);
  const auto volumeName = rowutil::requireString(source, "VolumeName", error, false);
  if (!path || !currentSession || !volumeName) return false;

  // UWF 也暴露恢复分区等无盘符卷；其 DriveLetter/Protected 是 VT_NULL，属于
  // provider 的合法对象，但本应用只管理可由 UI 映射的逻辑盘。字段缺失仍是
  // schema 错误，明确的 NULL 则跳过整行。
  const auto driveIt = source.find("DriveLetter");
  if (driveIt == source.end()) {
    if (error) *error = "WMI field 'DriveLetter' is missing";
    return false;
  }
  if (!driveIt->second.isValid()) return true;
  const auto driveLetter = rowutil::requireString(source, "DriveLetter", error);
  if (!driveLetter) return false;

  const auto normalizedDrive = drive::normalize(*driveLetter);
  if (normalizedDrive.empty()) return true;  // 合法但不属于本应用可管理的逻辑卷

  const auto bindByDriveLetter = rowutil::requireBool(source, "BindByDriveLetter", error);
  const auto commitPending = rowutil::requireBool(source, "CommitPending", error);
  const auto isProtected = rowutil::requireBool(source, "Protected", error);
  if (!bindByDriveLetter || !commitPending || !isProtected) return false;

  api::VolumeRow row;
  row.path = *path;
  row.currentSession = *currentSession;
  row.driveLetter = normalizedDrive;
  row.volumeName = *volumeName;
  row.bindByDriveLetter = *bindByDriveLetter;
  row.commitPending = *commitPending;
  row.isProtected = *isProtected;
  decoded = std::move(row);
  return true;
}

}  // namespace

std::vector<api::VolumeRow> UwfVolume::readAll(std::string* error) const {
  if (error) error->clear();
  std::vector<api::VolumeRow> out;
  std::string queryError;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Volume", &queryError);
  if (!queryError.empty()) {
    if (error) *error = std::move(queryError);
    return out;
  }

  size_t curCount = 0;
  size_t nextCount = 0;
  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_Volume", o);
    std::optional<api::VolumeRow> decoded;
    if (!decodeManagedVolume(o, decoded, error)) return {};
    if (!decoded) continue;
    if (decoded->currentSession)
      ++curCount;
    else
      ++nextCount;
    out.push_back(std::move(*decoded));
  }
  UWF_LOG_I("UWF_Volume") << std::format("readAll ok: rows={} (current={}, next={})", rows.size(), curCount, nextCount);
  return out;
}

namespace {

std::optional<api::VolumeRow> rereadVolume(WmiSession& session, const api::VolumeRow& target, std::string* error) {
  const auto source = session.getObject(target.path, error);
  if (!source) return std::nullopt;
  std::optional<api::VolumeRow> observed;
  if (!decodeManagedVolume(*source, observed, error)) return std::nullopt;
  if (!observed || observed->currentSession != target.currentSession || observed->driveLetter != target.driveLetter ||
      observed->volumeName != target.volumeName) {
    if (error) *error = "UWF_Volume reread returned a different instance identity";
    return std::nullopt;
  }
  return observed;
}

}  // namespace

WmiResult UwfVolume::protectVolume(const api::VolumeRow& row) const {
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "Protect"));
  std::string readError;
  const auto observed = rereadVolume(m_session, row, &readError);
  auto verification = verifyObservedState(observed, [](const api::VolumeRow& state) { return state.isProtected; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_Volume::Protect");
  if (out.ok) UWF_LOG_I("UWF_Volume") << "Protect confirmed: dl=" << row.driveLetter;
  return out;
}

WmiResult UwfVolume::unprotect(const api::VolumeRow& row) const {
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "Unprotect"));
  std::string readError;
  const auto observed = rereadVolume(m_session, row, &readError);
  auto verification = verifyObservedState(observed, [](const api::VolumeRow& state) { return !state.isProtected; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_Volume::Unprotect");
  if (out.ok) UWF_LOG_I("UWF_Volume") << "Unprotect confirmed: dl=" << row.driveLetter;
  return out;
}

WmiResult UwfVolume::removeAllExclusions(const api::VolumeRow& row) const {
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "RemoveAllExclusions"));
  std::string readError;
  const auto observed = getExclusions(row, &readError);
  auto verification = verifyObservedState(observed, [](const std::vector<api::ExcludedFile>& exclusions) { return exclusions.empty(); }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_Volume::RemoveAllExclusions");
  if (out.ok) UWF_LOG_I("UWF_Volume") << "RemoveAllExclusions confirmed: dl=" << row.driveLetter;
  return out;
}

// CommitFile / CommitFileDeletion 共享前置校验 + WMI 调用骨架——除方法名外完全
// 一致（路径剥盘符 → 校验 __PATH → 用 FileName 参数发 ExecMethod → 归类）。
// 仿 UwfRegistryFilter::invokeCommit 的写法抽到一处，两个 public 方法各 1 行
// 委托。实机 WMI schema 的参数名是 FileName（见 11-uwf-api.html 附录 B.1）。
WmiResult UwfVolume::invokeFileCommit(const api::VolumeRow& row, const std::string& fileFullPath, const char* method) const {
  std::string stripErr;
  const auto normalized = stripVolumeDriveLetter(fileFullPath, row.driveLetter, &stripErr);
  if (!normalized) {
    UWF_LOG_E("UWF_Volume") << method << " rejected: " << stripErr;
    return WmiResult::failed(std::move(stripErr));
  }
  if (row.path.empty()) {
    UWF_LOG_E("UWF_Volume") << method << " rejected: empty __PATH; file=" << fileFullPath;
    return WmiResult::failed("UWF_Volume row has empty __PATH; call readAll() first");
  }

  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, method, in));
  if (out.ok) UWF_LOG_I("UWF_Volume") << std::format("{} ok: dl={} file={}", method, row.driveLetter, *normalized);
  return out;
}

WmiResult UwfVolume::commitFile(const api::VolumeRow& row, const std::string& fileFullPath) const { return invokeFileCommit(row, fileFullPath, "CommitFile"); }

WmiResult UwfVolume::commitFileDeletion(const api::VolumeRow& row, const std::string& fileName) const {
  return invokeFileCommit(row, fileName, "CommitFileDeletion");
}

WmiResult UwfVolume::setBindByDriveLetter(const api::VolumeRow& row, bool bBindByDriveLetter) const {
  WmiRow in;
  in.emplace("bBindByDriveLetter", WmiValue::fromBool(bBindByDriveLetter));
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "SetBindByDriveLetter", in));
  std::string readError;
  const auto observed = rereadVolume(m_session, row, &readError);
  auto verification = verifyObservedState(
      observed, [bBindByDriveLetter](const api::VolumeRow& state) { return state.bindByDriveLetter == bBindByDriveLetter; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_Volume::SetBindByDriveLetter");
  if (out.ok) UWF_LOG_I("UWF_Volume") << std::format("SetBindByDriveLetter confirmed: dl={} bindByDriveLetter={}", row.driveLetter, bBindByDriveLetter);
  return out;
}

namespace {

// AddExclusion / RemoveExclusion 共用：路径剥盘符 + 单个 FileName 参数。
WmiResult invokeFileExclusion(WmiSession& session, const api::VolumeRow& row, const char* method, const std::string& fileName, const bool expectedFound) {
  std::string stripErr;
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, &stripErr);
  if (!normalized) return WmiResult::failed(std::move(stripErr));
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method, in));
  std::string readError;
  const auto observed = UwfVolume(session).findExclusion(row, *normalized, &readError);
  auto verification = verifyObservedState(observed, [expectedFound](const bool found) { return found == expectedFound; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), std::format("UWF_Volume::{}", method));
  if (out.ok) UWF_LOG_I("UWF_Volume") << std::format("{} confirmed: dl={} file={}", method, row.driveLetter, *normalized);
  return out;
}

}  // namespace

WmiResult UwfVolume::addExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  return invokeFileExclusion(m_session, row, "AddExclusion", fileName, true);
}

WmiResult UwfVolume::removeExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  return invokeFileExclusion(m_session, row, "RemoveExclusion", fileName, false);
}

std::optional<bool> UwfVolume::findExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error) const {
  if (error) error->clear();
  if (row.path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    return std::nullopt;
  }
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, error);
  if (!normalized) return std::nullopt;
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const auto r = m_session.callMethodRead(row.path, "FindExclusion", in);
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_Volume::FindExclusion");
    return std::nullopt;
  }
  return rowutil::requireBool(r.outParams, "bFound", error);
}

std::optional<std::vector<api::ExcludedFile>> UwfVolume::getExclusions(const api::VolumeRow& row, std::string* error) const {
  if (error) error->clear();
  if (row.path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    return std::nullopt;
  }
  const auto r = m_session.callMethodRead(row.path, "GetExclusions");
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_Volume::GetExclusions");
    return std::nullopt;
  }
  auto out = rowutil::readArrayOutput<api::ExcludedFile>(
      r, "ExcludedFiles",
      [error](const WmiRow& item) -> std::optional<api::ExcludedFile> {
        api::ExcludedFile e;
        const auto fileName = rowutil::requireEmbeddedString(item, "FileName", error);
        if (!fileName) return std::nullopt;
        e.fileName = *fileName;
        return e;
      },
      error);
  if (!out) return std::nullopt;
  UWF_LOG_D("UWF_Volume") << std::format("GetExclusions ok: dl={} session={} count={}", row.driveLetter, row.currentSession ? "current" : "next", out->size());
  return out;
}

EnsureVolumeResult UwfVolume::ensureNextSessionEntry(const std::string& driveLetter) const {
  EnsureVolumeResult result;
  const std::string normalizedDrive = drive::normalize(driveLetter);
  if (normalizedDrive.empty()) {
    result.error = std::format("ensureNextSessionEntry requires a valid drive letter: {}", driveLetter);
    return result;
  }

  std::string readError;
  const auto rows = readAll(&readError);
  if (!readError.empty()) {
    result.error = std::move(readError);
    return result;
  }

  // 1) 已有 next session 实例 → 直接复用，避免重复 PutInstance。
  for (const auto& v : rows) {
    if (!v.currentSession && v.driveLetter == normalizedDrive) {
      result.entry = v;
      return result;
    }
  }

  // 2) 没有 next session 实例：从同卷的 current session 实例上拿 VolumeName。
  //    UWF 自己写出的 current 实例 VolumeName 格式必然规范，避免再走
  //    Win32_Volume 然后归一化的路。
  std::string volumeName;
  for (const auto& v : rows) {
    if (v.currentSession && v.driveLetter == normalizedDrive) {
      volumeName = v.volumeName;
      break;
    }
  }
  if (volumeName.empty()) {
    result.error = std::format("no current-session UWF_Volume row found for {}; cannot register", normalizedDrive);
    return result;
  }

  // 3) PutInstance 创建 next session 实例（key = CurrentSession=false +
  //    DriveLetter + VolumeName）。
  WmiRow props;
  props.emplace("CurrentSession", WmiValue::fromBool(false));
  props.emplace("DriveLetter", WmiValue::fromString(normalizedDrive));
  props.emplace("VolumeName", WmiValue::fromString(volumeName));

  const auto put = m_session.putInstance("UWF_Volume", props, WmiPutMode::CreateOnly);
  result.writeAttempted = put.attempted;

  // 4) 用 key 三元组构造精确 relative path，并通过只读实例方法确认 provider
  //    已经能解析该对象。这里不依赖 ExecQuery 的可见性，也不使用 sleep/轮询。
  const std::string relativePath =
      std::format(R"(UWF_Volume.CurrentSession=FALSE,DriveLetter="{}",VolumeName="{}")", escapeWmiPathValue(normalizedDrive), escapeWmiPathValue(volumeName));

  const bool convergentExisting = WmiError(put.hresult) == WmiErrorCode::AlreadyExists;
  if (!put.ok && !convergentExisting && !put.outcomeUncertain) {
    result.error = put.detail;
    return result;
  }

  std::string verifyError;
  const auto source = m_session.getObject(relativePath, &verifyError);
  std::optional<api::VolumeRow> observed;
  const bool decoded = source && decodeManagedVolume(*source, observed, &verifyError);
  if (!decoded || !observed || observed->currentSession || observed->driveLetter != normalizedDrive || observed->volumeName != volumeName) {
    result.error = verifyError.empty() ? "created UWF_Volume instance does not match its requested identity" : std::move(verifyError);
    if (!put.detail.empty()) result.error += std::format("; original PutInstance result: {}", put.detail);
    return result;
  }
  // CreateOnly 成功，或传输结果不确定但目标实例已能精确读取，才能确认本次
  // 创建。AlreadyExists 只表示并发状态已经收敛，不把别人的创建记成本次写成功。
  result.writeConfirmed = put.ok || put.outcomeUncertain;
  result.entry = std::move(observed);
  UWF_LOG_I("UWF_Volume") << std::format("ensureNextSessionEntry confirmed from provider state: dl={} volumeName={}", normalizedDrive, volumeName);
  return result;
}

}  // namespace uwf::api
