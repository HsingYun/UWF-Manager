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

}  // namespace

std::vector<api::VolumeRow> UwfVolume::readAll(std::string* error) const {
  std::vector<api::VolumeRow> out;
  const auto rows = m_session.query("SELECT * FROM UWF_Volume", error);

  size_t curCount = 0;
  size_t nextCount = 0;
  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_Volume", o);
    api::VolumeRow r;
    r.path = rowutil::getString(o, "__PATH");
    r.currentSession = rowutil::getBool(o, "CurrentSession");
    r.driveLetter = drive::normalize(rowutil::getString(o, "DriveLetter"));
    r.volumeName = rowutil::getString(o, "VolumeName");
    r.bindByDriveLetter = rowutil::getBool(o, "BindByDriveLetter", true);
    r.commitPending = rowutil::getBool(o, "CommitPending");
    r.isProtected = rowutil::getBool(o, "Protected");
    if (r.currentSession)
      ++curCount;
    else
      ++nextCount;
    out.push_back(std::move(r));
  }
  UWF_LOG_I("UWF_Volume") << std::format("readAll ok: rows={} (current={}, next={})", rows.size(), curCount, nextCount);
  return out;
}

namespace {

// 无参 ExecMethod（Protect / Unprotect / RemoveAllExclusions）的共享骨架。
WmiResult invokeNoArg(const WmiSession& session, const api::VolumeRow& row, const char* method) {
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method));
  if (out.ok) UWF_LOG_I("UWF_Volume") << method << " ok: dl=" << row.driveLetter;
  return out;
}

}  // namespace

WmiResult UwfVolume::protectVolume(const api::VolumeRow& row) const { return invokeNoArg(m_session, row, "Protect"); }
WmiResult UwfVolume::unprotect(const api::VolumeRow& row) const { return invokeNoArg(m_session, row, "Unprotect"); }
WmiResult UwfVolume::removeAllExclusions(const api::VolumeRow& row) const { return invokeNoArg(m_session, row, "RemoveAllExclusions"); }

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
  if (out.ok) UWF_LOG_I("UWF_Volume") << std::format("SetBindByDriveLetter ok: dl={} bindByDriveLetter={}", row.driveLetter, bBindByDriveLetter);
  return out;
}

namespace {

// AddExclusion / RemoveExclusion 共用：路径剥盘符 + 单个 FileName 参数。
WmiResult invokeFileExclusion(const WmiSession& session, const api::VolumeRow& row, const char* method, const std::string& fileName) {
  std::string stripErr;
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, &stripErr);
  if (!normalized) return WmiResult::failed(std::move(stripErr));
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method, in));
  if (out.ok) UWF_LOG_I("UWF_Volume") << std::format("{} ok: dl={} file={}", method, row.driveLetter, *normalized);
  return out;
}

}  // namespace

WmiResult UwfVolume::addExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  return invokeFileExclusion(m_session, row, "AddExclusion", fileName);
}

WmiResult UwfVolume::removeExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  return invokeFileExclusion(m_session, row, "RemoveExclusion", fileName);
}

std::optional<bool> UwfVolume::findExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    return std::nullopt;
  }
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, error);
  if (!normalized) return std::nullopt;
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const auto r = m_session.callMethod(row.path, "FindExclusion", in);
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_Volume::FindExclusion");
    return std::nullopt;
  }
  return rowutil::getBool(r.outParams, "bFound");
}

std::vector<api::ExcludedFile> UwfVolume::getExclusions(const api::VolumeRow& row, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    return {};
  }
  const auto r = m_session.callMethod(row.path, "GetExclusions");
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_Volume::GetExclusions");
    return {};
  }
  auto out = rowutil::readOutArray<api::ExcludedFile>(r, "ExcludedFiles", [](const WmiRow& item) -> std::optional<api::ExcludedFile> {
    api::ExcludedFile e;
    e.fileName = rowutil::readExcludedKey(item, "FileName");
    if (e.fileName.empty()) return std::nullopt;
    return e;
  });
  UWF_LOG_D("UWF_Volume") << std::format("GetExclusions ok: dl={} session={} count={}", row.driveLetter, row.currentSession ? "current" : "next", out.size());
  return out;
}

std::optional<api::VolumeRow> UwfVolume::ensureNextSessionEntry(const std::string& driveLetter, std::string* error) const {
  if (driveLetter.empty()) {
    if (error) *error = "ensureNextSessionEntry requires a non-empty driveLetter";
    return std::nullopt;
  }

  const auto rows = readAll();

  // 1) 已有 next session 实例 → 直接复用，避免重复 PutInstance。
  for (const auto& v : rows) {
    if (!v.currentSession && v.driveLetter == driveLetter) return v;
  }

  // 2) 没有 next session 实例：从同卷的 current session 实例上拿 VolumeName。
  //    UWF 自己写出的 current 实例 VolumeName 格式必然规范，避免再走
  //    Win32_Volume 然后归一化的路。
  std::string volumeName;
  for (const auto& v : rows) {
    if (v.currentSession && v.driveLetter == driveLetter) {
      volumeName = v.volumeName;
      break;
    }
  }
  if (volumeName.empty()) {
    if (error) *error = std::format("no current-session UWF_Volume row found for {}; cannot register", driveLetter);
    return std::nullopt;
  }

  // 3) PutInstance 创建 next session 实例（key = CurrentSession=false +
  //    DriveLetter + VolumeName）。
  WmiRow props;
  props.emplace("CurrentSession", WmiValue::fromBool(false));
  props.emplace("DriveLetter", WmiValue::fromString(driveLetter));
  props.emplace("VolumeName", WmiValue::fromString(volumeName));

  if (const auto put = m_session.putInstance("UWF_Volume", props); !put.ok) {
    if (error) *error = put.detail;
    return std::nullopt;
  }
  UWF_LOG_I("UWF_Volume") << std::format("ensureNextSessionEntry created: dl={} volumeName={}", driveLetter, volumeName);

  // 4) PutInstance 是同步的，但 UWF provider 对 ExecQuery 的可见性略微滞后；
  //    立即 readAll 经常拿不到，要等下次 query 才出现。所以不依赖 readAll，
  //    按 key 三元组直接构造 relative __PATH 给 caller —— WMI 的 ExecMethod
  //    接受 relative path（不含 hostname / namespace），后续 Protect 等
  //    instance 方法能正确解析。
  api::VolumeRow row;
  row.currentSession = false;
  row.driveLetter = driveLetter;
  row.volumeName = volumeName;
  row.isProtected = false;
  row.bindByDriveLetter = true;
  row.path =
      std::format(R"(UWF_Volume.CurrentSession=FALSE,DriveLetter="{}",VolumeName="{}")", escapeWmiPathValue(driveLetter), escapeWmiPathValue(volumeName));
  return row;
}

}  // namespace uwf::api
