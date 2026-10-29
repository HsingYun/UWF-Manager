// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfVolume.h"

#include <algorithm>
#include <exception>
#include <format>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "WriteVerification.h"
#include "app/ports/VolumePaths.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"
#include "shared/StringUtil.h"

namespace uwf::app::provider {

namespace {

std::string stripVolumeDriveLetter(const std::string& path, const std::string& volumeDriveLetter) {
  const auto split = drive::split(path);
  if (split.letter.empty()) return path;
  if (split.letter != volumeDriveLetter) {
    throw std::invalid_argument(std::format("path '{}' belongs to {}, not target volume {}", path, split.letter, volumeDriveLetter));
  }
  return split.rest;
}

std::string escapeWmiPathValue(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size() + 8);
  for (const char character : value) {
    if (character == '\\' || character == '"') escaped += '\\';
    escaped += character;
  }
  return escaped;
}

std::optional<provider::VolumeRow> decodeManagedVolume(const wmi::WmiRow& source) {
  const std::string path = wmi::row::requireString(source, "__PATH", wmi::row::EmptyString::Reject);
  const bool currentSession = wmi::row::requireBool(source, "CurrentSession");
  const std::string volumeName = wmi::row::requireString(source, "VolumeName", wmi::row::EmptyString::Reject);

  const auto drive = source.find("DriveLetter");
  if (drive == source.end()) throw wmi::WmiDecodeError("decode UWF_Volume", "field 'DriveLetter' is missing");
  if (!drive->second.isValid()) return std::nullopt;
  const std::string driveLetter = wmi::row::requireString(source, "DriveLetter");
  if (driveLetter.empty()) return std::nullopt;
  const std::string normalizedDrive = drive::normalize(driveLetter);
  if (normalizedDrive.empty()) throw wmi::WmiDecodeError("decode UWF_Volume", std::format("invalid DriveLetter: {}", driveLetter));

  return provider::VolumeRow{path,
                             currentSession,
                             normalizedDrive,
                             volumeName,
                             wmi::row::requireBool(source, "BindByDriveLetter"),
                             wmi::row::requireBool(source, "CommitPending"),
                             wmi::row::requireBoolOrFalseIfNull(source, "Protected")};
}

provider::VolumeRow rereadVolume(wmi::WmiOperations& session, const provider::VolumeRow& target) {
  auto observed = decodeManagedVolume(session.getObject(target.path));
  if (!observed || observed->currentSession != target.currentSession || observed->driveLetter != target.driveLetter ||
      observed->volumeName != target.volumeName) {
    throw wmi::WmiProtocolError("reread UWF_Volume", "provider returned a different volume instance");
  }
  return *observed;
}

void requirePath(const provider::VolumeRow& row, const char* operation) {
  if (row.path.empty()) throw wmi::WmiProtocolError(operation, "UWF_Volume row has no object path");
}

std::string normalizeVolumePath(const provider::VolumeRow& row, const std::string& fileName) { return stripVolumeDriveLetter(fileName, row.driveLetter); }

wmi::WmiRow fileInput(const std::string& normalizedFileName) {
  wmi::WmiRow inputs;
  inputs.emplace("FileName", wmi::WmiValue::fromString(normalizedFileName));
  return inputs;
}

bool isAlreadyExists(const wmi::WmiInfrastructureError& error) {
  return wmi::WmiError(static_cast<int32_t>(error.code().value())) == wmi::WmiErrorCode::AlreadyExists;
}

}  // namespace

UwfVolume::UwfVolume(wmi::WmiOperations& session, MountLookup mounts) : m_session(session), m_mounts(std::move(mounts)) {
  if (!m_mounts) throw std::invalid_argument("UWF volume mount lookup is required");
}

std::vector<provider::VolumeRow> UwfVolume::readAll() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Volume");
  std::vector<provider::VolumeRow> volumes;
  std::set<std::tuple<bool, std::string, std::string>> identities;
  std::map<std::pair<bool, std::string>, std::vector<std::size_t>> sessions;
  volumes.reserve(rows.size());
  for (const auto& source : rows) {
    wmi::row::dumpRow("UWF_Volume", source);
    auto decoded = decodeManagedVolume(source);
    if (!decoded) continue;
    const auto volumeName = toUpperAscii(decoded->volumeName);
    if (!identities.emplace(decoded->currentSession, decoded->driveLetter, volumeName).second) {
      throw wmi::WmiProtocolError("read UWF_Volume", std::format("duplicate {} session instance for {} / {}", decoded->currentSession ? "current" : "next",
                                                                 decoded->driveLetter, decoded->volumeName));
    }
    sessions[{decoded->currentSession, decoded->driveLetter}].push_back(volumes.size());
    volumes.push_back(std::move(*decoded));
  }

  // 两个会话各自独立：单一配置完整保留，同一会话内的盘符冲突才需要消歧。
  // 不能用当前挂载关系抹掉另一会话中明确配置的卷身份。挂载缓存只活在这次
  // 读取中，不沿用磁盘拔插前的结论。
  std::map<std::string, std::vector<std::string>> mountedLetters;
  const auto mountedAtDrive = [&](const provider::VolumeRow& row) {
    const auto key = toUpperAscii(row.volumeName);
    auto mounts = mountedLetters.find(key);
    if (mounts == mountedLetters.end()) {
      mounts = mountedLetters.emplace(key, m_mounts(R"(\\?\)" + row.volumeName + '\\')).first;
    }
    return std::ranges::find(mounts->second, row.driveLetter) != mounts->second.end();
  };
  std::vector<bool> selected(volumes.size(), true);
  for (const auto& [identity, indices] : sessions) {
    const auto& [currentSession, driveLetter] = identity;
    if (indices.size() == 1) continue;
    std::optional<std::size_t> match;
    for (const auto index : indices) {
      const auto& row = volumes[index];
      // 当前状态必须属于实际挂载卷；下次会话按盘符绑定的配置则仍针对该
      // 盘符生效，不能仅因记录中的旧 GUID 不再挂载就丢弃。
      const bool applies = (!currentSession && row.bindByDriveLetter) || mountedAtDrive(row);
      selected[index] = applies;
      if (!applies) continue;
      if (match) {
        throw wmi::WmiProtocolError("resolve UWF_Volume", std::format("ambiguous {} session instances for {}: {} and {}", currentSession ? "current" : "next",
                                                                      driveLetter, volumes[*match].volumeName, row.volumeName));
      }
      match = index;
    }
  }
  std::vector<provider::VolumeRow> managed;
  managed.reserve(volumes.size());
  for (std::size_t index = 0; index < volumes.size(); ++index) {
    if (selected[index]) managed.push_back(std::move(volumes[index]));
  }
  UWF_LOG_D("uwf") << "volume read completed: providerRows=" << rows.size() << " managedRows=" << managed.size();
  return managed;
}

void UwfVolume::protectVolume(const provider::VolumeRow& row) const {
  requirePath(row, "protect UWF volume");
  invokeAndConfirm("protect UWF volume", [&] { m_session.invokeMethod(row.path, "Protect"); }, [&] { return rereadVolume(m_session, row).isProtected; });
}

void UwfVolume::unprotect(const provider::VolumeRow& row) const {
  requirePath(row, "unprotect UWF volume");
  invokeAndConfirm("unprotect UWF volume", [&] { m_session.invokeMethod(row.path, "Unprotect"); }, [&] { return !rereadVolume(m_session, row).isProtected; });
}

void UwfVolume::commitFile(const provider::VolumeRow& row, const std::string& fileFullPath) const {
  requirePath(row, "commit UWF file");
  const wmi::WmiRow inputs = fileInput(normalizeVolumePath(row, fileFullPath));
  m_session.invokeMethod(row.path, "CommitFile", inputs);
}

void UwfVolume::commitFileDeletion(const provider::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "commit UWF file deletion");
  const wmi::WmiRow inputs = fileInput(normalizeVolumePath(row, fileName));
  m_session.invokeMethod(row.path, "CommitFileDeletion", inputs);
}

void UwfVolume::setBinding(const provider::VolumeRow& row, const provider::VolumeBinding binding) const {
  requirePath(row, "set UWF volume binding");
  const bool bindByDriveLetter = binding == provider::VolumeBinding::DriveLetter;
  wmi::WmiRow inputs;
  inputs.emplace("bBindByDriveLetter", wmi::WmiValue::fromBool(bindByDriveLetter));
  invokeAndConfirm(
      "set UWF volume binding", [&] { m_session.invokeMethod(row.path, "SetBindByDriveLetter", inputs); },
      [&] { return rereadVolume(m_session, row).bindByDriveLetter == bindByDriveLetter; });
}

void UwfVolume::addExclusion(const provider::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "add UWF file exclusion");
  const std::string normalized = normalizeVolumePath(row, fileName);
  const wmi::WmiRow inputs = fileInput(normalized);
  invokeAndConfirm("add UWF file exclusion", [&] { m_session.invokeMethod(row.path, "AddExclusion", inputs); }, [&] { return findExclusion(row, normalized); });
}

void UwfVolume::removeExclusion(const provider::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "remove UWF file exclusion");
  const std::string normalized = normalizeVolumePath(row, fileName);
  const wmi::WmiRow inputs = fileInput(normalized);
  invokeAndConfirm(
      "remove UWF file exclusion", [&] { m_session.invokeMethod(row.path, "RemoveExclusion", inputs); }, [&] { return !findExclusion(row, normalized); });
}

void UwfVolume::removeAllExclusions(const provider::VolumeRow& row) const {
  requirePath(row, "remove all UWF file exclusions");
  invokeAndConfirm(
      "remove all UWF file exclusions", [&] { m_session.invokeMethod(row.path, "RemoveAllExclusions"); }, [&] { return getExclusions(row).empty(); });
}

bool UwfVolume::findExclusion(const provider::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "find UWF file exclusion");
  const wmi::WmiRow inputs = fileInput(normalizeVolumePath(row, fileName));
  const auto output = m_session.callMethodRead(row.path, "FindExclusion", inputs);
  return wmi::row::requireBool(output.values, "bFound");
}

std::vector<provider::ExcludedFile> UwfVolume::getExclusions(const provider::VolumeRow& row) const {
  requirePath(row, "read UWF file exclusions");
  const auto output = m_session.callMethodRead(row.path, "GetExclusions");
  return wmi::row::readArrayOutput<provider::ExcludedFile>(
      output, "ExcludedFiles", [](const wmi::WmiRow& item) { return provider::ExcludedFile{wmi::row::requireEmbeddedString(item, "FileName")}; });
}

VolumeRegistration UwfVolume::ensureNextSessionEntry(const std::string& driveLetter) const {
  const std::string normalizedDrive = drive::normalize(driveLetter);
  if (normalizedDrive.empty()) throw std::invalid_argument(std::format("invalid drive letter: {}", driveLetter));

  const auto volumes = readAll();
  if (const auto* existing =
          provider::findBySession(volumes, provider::Session::Next, [&](const provider::VolumeRow& volume) { return volume.driveLetter == normalizedDrive; })) {
    return {*existing, VolumeRegistrationDisposition::AlreadyPresent};
  }
  const auto* current =
      provider::findBySession(volumes, provider::Session::Current, [&](const provider::VolumeRow& volume) { return volume.driveLetter == normalizedDrive; });
  if (!current) throw wmi::WmiProtocolError("register UWF volume", std::format("no current-session instance exists for {}", normalizedDrive));

  wmi::WmiRow properties;
  properties.emplace("CurrentSession", wmi::WmiValue::fromBool(false));
  properties.emplace("DriveLetter", wmi::WmiValue::fromString(normalizedDrive));
  properties.emplace("VolumeName", wmi::WmiValue::fromString(current->volumeName));

  VolumeRegistrationDisposition outcome = VolumeRegistrationDisposition::Created;
  std::string uncertainFailure;
  try {
    m_session.putInstance("UWF_Volume", properties, wmi::WmiPutMode::CreateOnly);
  } catch (const wmi::WmiInfrastructureError& error) {
    if (isAlreadyExists(error)) {
      outcome = VolumeRegistrationDisposition::ConcurrentlyCreated;
    } else {
      throw;
    }
  } catch (const wmi::WmiInvocationUncertain& error) {
    uncertainFailure = error.what();
    outcome = VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite;
  }

  const std::string relativePath = std::format(R"(UWF_Volume.CurrentSession=FALSE,DriveLetter="{}",VolumeName="{}")", escapeWmiPathValue(normalizedDrive),
                                               escapeWmiPathValue(current->volumeName));
  std::optional<provider::VolumeRow> observed;
  try {
    observed = decodeManagedVolume(m_session.getObject(relativePath));
  } catch (const std::exception& confirmationError) {
    std::string detail;
    if (outcome == VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite) {
      detail = "write outcome was uncertain and the created instance could not be confirmed; original failure: " + uncertainFailure;
    } else if (outcome == VolumeRegistrationDisposition::ConcurrentlyCreated) {
      detail = "the instance was concurrently created, but its state could not be confirmed";
    } else {
      detail = "provider accepted the registration, but the created instance could not be confirmed";
    }
    detail += "; reread failure: ";
    detail += confirmationError.what();
    std::throw_with_nested(wmi::WmiStateVerificationError("register UWF volume", detail));
  } catch (...) {
    const std::string detail =
        outcome == VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite
            ? "write outcome was uncertain and state confirmation failed with a non-standard exception; original failure: " + uncertainFailure
            : "registration state confirmation failed with a non-standard exception";
    std::throw_with_nested(wmi::WmiStateVerificationError("register UWF volume", detail));
  }

  if (!observed || observed->currentSession || observed->driveLetter != normalizedDrive || observed->volumeName != current->volumeName) {
    std::string detail;
    if (outcome == VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite) {
      detail =
          "write outcome was uncertain and the observed instance does not match the requested next-session identity; original failure: " + uncertainFailure;
    } else if (outcome == VolumeRegistrationDisposition::ConcurrentlyCreated) {
      detail = "the concurrently created instance does not match the requested next-session identity";
    } else {
      detail = "provider accepted the registration, but the observed instance does not match the requested next-session identity";
    }
    throw wmi::WmiStateVerificationError("register UWF volume", detail);
  }
  return {std::move(*observed), outcome};
}

}  // namespace uwf::app::provider
