// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/exclusions/ExclusionRules.h"

#include <QStringView>
#include <cstddef>

#include "application/localization/ApplicationText.h"
#include "application/paths/PathPolicy.h"
#include "domain/uwf/policies/FileExclusionPolicy.h"
#include "domain/uwf/policies/RegistryExclusionPolicy.h"
#include "domain/uwf/policies/UwfConstraints.h"
#include "foundation/paths/DriveLetter.h"
#include "foundation/text/StringUtil.h"
#include "foundation/text/Utf8.h"

namespace uwf::app {

namespace {
QLatin1String l1(std::string_view sv) { return QLatin1String(sv.data(), static_cast<qsizetype>(sv.size())); }
}  // namespace

QString FileExclusionRules::rejectionReason(const QString& rawPath, const QString& volumeDriveLetter, const QString& systemDrive) {
  if (rawPath.contains(QChar::Null)) return ApplicationText::tr("File paths cannot contain null characters.");
  if (!isValidUtf16(QStringView(rawPath))) return ApplicationText::tr("The exclusion path contains characters that UWF does not support.");
  if (rawPath.contains(u'/')) return ApplicationText::tr("UWF exclusion paths must use backslashes.");
  const QString volume = QString::fromStdString(drive::normalize(volumeDriveLetter.toStdString()));
  const QString upper = QString::fromStdString(toUpperAscii(paths::normalize(rawPath, volume).toStdString()));
  if (upper.isEmpty()) return {};

  if (upper == volume || upper == volume + "\\") {
    return ApplicationText::tr("Cannot add an entire volume (%1) to the exclusion list.").arg(volume);
  }

  // Validate the exact volume-relative string sent by the repository. Drive
  // qualification is transport addressing, not permission to clean the pattern.
  const auto original = rawPath.toStdString();
  const auto address = drive::split(original);
  const QString relative = QString::fromStdString(address.letter.empty() ? original : address.rest);
  const auto error = core::fileExclusionError({QStringView(relative).utf16(), static_cast<std::size_t>(relative.size())});
  if (error == core::FileExclusionError::TooLong) return ApplicationText::tr("The exclusion path exceeds UWF's length limit.");
  if (error != core::FileExclusionError::None) return ApplicationText::tr("The exclusion path contains characters that UWF does not support.");

  QString rel = upper;
  if (rel.startsWith(volume)) rel = rel.mid(volume.size());
  if (!rel.startsWith("\\")) rel = "\\" + rel;
  while (rel.size() > 1 && rel.endsWith("\\")) rel.chop(1);

  for (const auto f : core::kForbiddenVolumeRootFiles) {
    if (rel == l1(f)) {
      return ApplicationText::tr("The pagefile, swapfile and hibernation file cannot be added as exclusions.");
    }
  }

  const bool isSystemVolume = !volume.isEmpty() && volume == QString::fromStdString(drive::normalize(systemDrive.toStdString()));
  if (!isSystemVolume) return {};

  // Only the directory itself is reserved; individual descendants may be excluded.
  if (rel == l1(core::kForbiddenDirWindows)) return ApplicationText::tr("The entire \\Windows directory cannot be excluded.");
  if (rel == l1(core::kForbiddenDirWindowsSystem32)) return ApplicationText::tr("The entire \\Windows\\System32 directory cannot be excluded.");
  if (rel == l1(core::kForbiddenDirWindowsDrivers)) return ApplicationText::tr(R"(The entire \Windows\System32\Drivers directory cannot be excluded.)");

  for (const auto f : core::kForbiddenSystemFiles) {
    if (rel == l1(f)) return ApplicationText::tr("This critical system file cannot be excluded: %1").arg(l1(f).toString().toLower());
  }

  // Match the per-user hive at exactly one username component below Users.
  const QString usersDir = l1(core::kUsersDirName).toString();
  const QString ntuserDat = l1(core::kPerUserRegistryHive).toString();
  if (rel.startsWith('\\' + usersDir + '\\') && rel.endsWith('\\' + ntuserDat)) {
    const auto parts = rel.split('\\', Qt::SkipEmptyParts);
    if (parts.size() == 3 && parts[0] == usersDir && parts[2] == ntuserDat) {
      return ApplicationText::tr("The per-user registry file NTUSER.DAT cannot be excluded.");
    }
  }

  return {};
}

// Preserve whitespace because it may belong to the actual key name.
QString RegistryExclusionRules::rejectionReason(const QString& rawKey) {
  const QString& input = rawKey;
  if (input.isEmpty()) return ApplicationText::tr("Registry key cannot be empty.");
  if (!isValidUtf16(QStringView(input))) return ApplicationText::tr("The exclusion path contains characters that UWF does not support.");

  const auto error = core::registryExclusionError({QStringView(input).utf16(), static_cast<std::size_t>(input.size())});
  switch (error) {
    case core::RegistryExclusionError::EmbeddedNull:
      return ApplicationText::tr("Registry paths cannot contain null characters.");
    case core::RegistryExclusionError::ComponentTooLong:
      return ApplicationText::tr("A registry key component is too long.");
    case core::RegistryExclusionError::TooManyComponents:
      return ApplicationText::tr("The registry path exceeds UWF's nesting limit.");
    case core::RegistryExclusionError::MachineSubkeyRequired:
      if (input.startsWith(u'\\')) return ApplicationText::tr("Registry key cannot start with a backslash.");
      if (core::classifyRegistryExclusion(input.toStdString()) == core::RegExclusionClass::Container)
        return ApplicationText::tr("This key cannot be excluded itself. Select one of its subkeys.");
      return ApplicationText::tr("Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).");
    case core::RegistryExclusionError::Reserved:
      return ApplicationText::tr("This key cannot be excluded itself. Select one of its subkeys.");
    case core::RegistryExclusionError::None:
      break;
  }

  switch (core::classifyRegistryExclusion(input.toStdString())) {
    case core::RegExclusionClass::Allowed:
      return {};
    case core::RegExclusionClass::MachineAccount:
      return ApplicationText::tr("Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).");
    case core::RegExclusionClass::Container:
      return ApplicationText::tr("This key cannot be excluded itself. Select one of its subkeys.");
    case core::RegExclusionClass::OutOfScope:
      return ApplicationText::tr("Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).");
  }
  return {};
}

}  // namespace uwf::app
