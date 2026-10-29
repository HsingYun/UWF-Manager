// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ExclusionRules.h"

#include <utility>

#include "app/common/ApplicationText.h"
#include "app/paths/PathPolicy.h"
#include "app/ports/VolumePaths.h"
#include "core/RegistryExclusionPolicy.h"
#include "core/UwfConstraints.h"

namespace uwf::app {

namespace {
QLatin1String l1(std::string_view sv) { return QLatin1String(sv.data(), static_cast<qsizetype>(sv.size())); }
}  // namespace

// 返回禁止排除路径的本地化原因；空串表示允许。
QString FileExclusionRules::rejectionReason(const QString& rawPath, const QString& volumeDl) {
  const QString volume = QString::fromStdString(drive::normalize(volumeDl.toStdString()));
  const QString upper = paths::normalize(rawPath.trimmed(), volume).toUpper();
  if (upper.isEmpty()) return {};

  // 卷根本身：match "C:" 或 "C:\"。
  if (upper == volume || upper == volume + "\\") {
    return ApplicationText::tr("Cannot add an entire volume (%1) to the exclusion list.").arg(volume);
  }

  // 剥掉盘符前缀，得到以 "\" 开头的卷内相对路径。
  QString rel = upper;
  if (rel.startsWith(volume)) rel = rel.mid(volume.size());
  if (!rel.startsWith("\\")) rel = "\\" + rel;
  while (rel.size() > 1 && rel.endsWith("\\")) rel.chop(1);

  // 分页/交换/休眠文件——放在卷根上的系统文件；任何卷都禁。
  for (const auto f : core::kForbiddenVolumeRootFiles) {
    if (rel == l1(f)) {
      return ApplicationText::tr("The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.");
    }
  }

  const bool isSystemVolume = (volume == QString::fromStdString(drive::systemLetter()));
  if (!isSystemVolume) return {};

  // 系统卷：几个关键目录本身不能排除（里面的具体文件可以）。
  if (rel == l1(core::kForbiddenDirWindows)) return ApplicationText::tr("The entire \\Windows directory cannot be excluded.");
  if (rel == l1(core::kForbiddenDirWindowsSystem32)) return ApplicationText::tr("The entire \\Windows\\System32 directory cannot be excluded.");
  if (rel == l1(core::kForbiddenDirWindowsDrivers)) return ApplicationText::tr(R"(The entire \Windows\System32\Drivers directory cannot be excluded.)");

  // 系统卷：几个关键文件（注册表蜂巢 / 引导统计）。
  for (const auto f : core::kForbiddenSystemFiles) {
    if (rel == l1(f)) return ApplicationText::tr("This critical system file cannot be excluded: %1").arg(l1(f).toString().toLower());
  }

  // 每个用户的 NTUSER.DAT：\Users\<User Name>\NTUSER.DAT，用户名任意但
  // 只有一层。
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

// 语法检查在归一化前执行；排除资格与 RegistryPickerDialog 共用 classifyRegistryExclusion。
QString RegistryExclusionRules::rejectionReason(const QString& rawKey) {
  QString input = rawKey.trimmed();
  while (input.size() > 1 && input.endsWith('\\')) input.chop(1);
  if (input.isEmpty()) return ApplicationText::tr("Registry key cannot be empty.");

  if (input.startsWith('\\')) return ApplicationText::tr("Registry key cannot start with a backslash.");
  if (input.contains("\\\\")) return ApplicationText::tr("The path contains consecutive backslashes; this is not valid.");
  if (input.contains('/')) return ApplicationText::tr("Registry paths use backslash `\\` as the separator; do not use forward slash `/`.");
  for (QChar c : std::as_const(input)) {
    if (c.unicode() < 0x20) return ApplicationText::tr("The path contains invisible control characters; this is not valid.");
  }

  switch (core::classifyRegistryExclusion(input.toStdString())) {
    case core::RegExclusionClass::Allowed:
      return {};
    case core::RegExclusionClass::MachineAccount:
      return ApplicationText::tr(
          "HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly "
          "forbids excluding.");
    case core::RegExclusionClass::Container:
    case core::RegExclusionClass::OutOfScope:
      return ApplicationText::tr(
          "UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:\n  HKLM\\BCD00000000\n  "
          "HKLM\\SYSTEM\n  HKLM\\SOFTWARE\n  HKLM\\SAM\n  HKLM\\SECURITY\n  HKLM\\COMPONENTS");
  }
  return {};
}

}  // namespace uwf::app
