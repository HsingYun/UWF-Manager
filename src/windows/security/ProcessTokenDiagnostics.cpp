// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ProcessTokenDiagnostics.h"

#include "windows/system/WindowsErrorText.h"

// MinGW 的 sddl.h 依赖 windows.h 先定义 WINADVAPI / WINBOOL。
// clang-format off
#include <windows.h>
#include <sddl.h>
// clang-format on

#include <QScopeGuard>
#include <QStringList>
#include <cstddef>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "windows/resources/UniqueHandle.h"

namespace uwf::windows {

namespace {

using TokenInformationResult = std::variant<std::vector<BYTE>, DWORD>;
using TextQueryResult = std::variant<QString, DWORD>;

QString yesNo(const bool value) { return value ? QStringLiteral("yes") : QStringLiteral("no"); }

TokenInformationResult tokenInformation(const HANDLE token, const TOKEN_INFORMATION_CLASS kind) {
  DWORD bytes = 0;
  if (GetTokenInformation(token, kind, nullptr, 0, &bytes)) return std::vector<BYTE>{};

  const DWORD sizeQueryError = GetLastError();
  if (sizeQueryError != ERROR_INSUFFICIENT_BUFFER) return sizeQueryError;

  std::vector<BYTE> result(bytes);
  if (!GetTokenInformation(token, kind, result.data(), bytes, &bytes)) {
    const DWORD queryError = GetLastError();
    return queryError;
  }
  return result;
}

void addValue(std::vector<app::DiagnosticField>& fields, QString key, QString value) { fields.push_back({std::move(key), std::move(value)}); }

TextQueryResult accountName(const PSID sid) {
  DWORD nameSize = 0;
  DWORD domainSize = 0;
  SID_NAME_USE use{};
  if (!LookupAccountSidW(nullptr, sid, nullptr, &nameSize, nullptr, &domainSize, &use)) {
    const DWORD sizeQueryError = GetLastError();
    if (sizeQueryError != ERROR_INSUFFICIENT_BUFFER) return sizeQueryError;
  }
  if (nameSize == 0) return static_cast<DWORD>(ERROR_INVALID_DATA);
  std::wstring name(nameSize, L'\0');
  std::wstring domain(domainSize, L'\0');
  if (!LookupAccountSidW(nullptr, sid, name.data(), &nameSize, domain.data(), &domainSize, &use)) {
    const DWORD queryError = GetLastError();
    return queryError;
  }
  name.resize(nameSize);
  domain.resize(domainSize);
  const QString qName = QString::fromStdWString(name);
  return domain.empty() ? qName : QStringLiteral("%1\\%2").arg(QString::fromStdWString(domain), qName);
}

TextQueryResult sidText(const PSID sid) {
  wchar_t* raw = nullptr;
  if (!ConvertSidToStringSidW(sid, &raw)) {
    const DWORD error = GetLastError();
    return error;
  }
  if (!raw) return static_cast<DWORD>(ERROR_INVALID_DATA);
  const auto releaseSid = qScopeGuard([raw] { LocalFree(raw); });
  return QString::fromWCharArray(raw);
}

void addTextQueryResult(std::vector<app::DiagnosticField>& fields, const QString& key, const TextQueryResult& result) {
  if (const auto* value = std::get_if<QString>(&result))
    addValue(fields, key, *value);
  else
    addValue(fields, key + QStringLiteral(".error"), windowsErrorText(std::get<DWORD>(result)));
}

QString elevationTypeText(const TOKEN_ELEVATION_TYPE type) {
  switch (type) {
    case TokenElevationTypeDefault:
      return QStringLiteral("default");
    case TokenElevationTypeFull:
      return QStringLiteral("full");
    case TokenElevationTypeLimited:
      return QStringLiteral("limited");
  }
  return QStringLiteral("unknown");
}

QString integrityLevelText(const DWORD rid) {
  if (rid < SECURITY_MANDATORY_LOW_RID) return QStringLiteral("untrusted (%1)").arg(rid);
  if (rid < SECURITY_MANDATORY_MEDIUM_RID) return QStringLiteral("low (%1)").arg(rid);
  if (rid < SECURITY_MANDATORY_HIGH_RID) return QStringLiteral("medium (%1)").arg(rid);
  if (rid < SECURITY_MANDATORY_SYSTEM_RID) return QStringLiteral("high (%1)").arg(rid);
  if (rid < SECURITY_MANDATORY_PROTECTED_PROCESS_RID) return QStringLiteral("system (%1)").arg(rid);
  return QStringLiteral("protected-process (%1)").arg(rid);
}

QString privilegeName(const LUID luid) {
  LUID queryLuid = luid;
  DWORD size = 0;
  LookupPrivilegeNameW(nullptr, &queryLuid, nullptr, &size);
  if (size == 0) return {};
  std::wstring name(static_cast<std::size_t>(size) + 1, L'\0');
  if (!LookupPrivilegeNameW(nullptr, &queryLuid, name.data(), &size)) return {};
  name.resize(size);
  return QString::fromStdWString(name);
}

QString privilegeState(const DWORD attributes) {
  QStringList states;
  states.append((attributes & SE_PRIVILEGE_ENABLED) != 0 ? QStringLiteral("enabled") : QStringLiteral("disabled"));
  if ((attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT) != 0) states.append(QStringLiteral("enabled-by-default"));
  if ((attributes & SE_PRIVILEGE_REMOVED) != 0) states.append(QStringLiteral("removed"));
  if ((attributes & SE_PRIVILEGE_USED_FOR_ACCESS) != 0) states.append(QStringLiteral("used-for-access"));
  return states.join(QStringLiteral(", "));
}

template <typename Value>
void appendFixedTokenValue(std::vector<app::DiagnosticField>& fields, const HANDLE token, const TOKEN_INFORMATION_CLASS kind, const QString& key, Value& value,
                           const auto& formatter) {
  DWORD returned = 0;
  if (GetTokenInformation(token, kind, &value, sizeof(value), &returned)) {
    addValue(fields, key, formatter(value));
    return;
  }
  const DWORD error = GetLastError();
  addValue(fields, key + QStringLiteral(".error"), windowsErrorText(error));
}

}  // namespace

std::vector<app::DiagnosticField> collectProcessTokenDiagnostics() {
  std::vector<app::DiagnosticField> fields;
  HANDLE rawToken = nullptr;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &rawToken)) {
    const DWORD error = GetLastError();
    addValue(fields, QStringLiteral("token.error"), windowsErrorText(error));
    return fields;
  }
  const windows::UniqueHandle token(rawToken);

  const TokenInformationResult userResult = tokenInformation(token.get(), TokenUser);
  if (const auto* user = std::get_if<std::vector<BYTE>>(&userResult); user && user->size() >= sizeof(TOKEN_USER)) {
    const auto* tokenUser = reinterpret_cast<const TOKEN_USER*>(user->data());
    addTextQueryResult(fields, QStringLiteral("user.account"), accountName(tokenUser->User.Sid));
    addTextQueryResult(fields, QStringLiteral("user.sid"), sidText(tokenUser->User.Sid));
  } else if (const auto* error = std::get_if<DWORD>(&userResult)) {
    addValue(fields, QStringLiteral("user.error"), windowsErrorText(*error));
  } else {
    addValue(fields, QStringLiteral("user.error"), QStringLiteral("TokenUser returned no data"));
  }

  TOKEN_ELEVATION elevation{};
  appendFixedTokenValue(fields, token.get(), TokenElevation, QStringLiteral("elevated"), elevation,
                        [](const TOKEN_ELEVATION& value) { return yesNo(value.TokenIsElevated != 0); });
  TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
  appendFixedTokenValue(fields, token.get(), TokenElevationType, QStringLiteral("elevation.type"), elevationType, elevationTypeText);
  DWORD sessionId = 0;
  appendFixedTokenValue(fields, token.get(), TokenSessionId, QStringLiteral("session.id"), sessionId, [](const DWORD value) { return QString::number(value); });

  const TokenInformationResult integrityResult = tokenInformation(token.get(), TokenIntegrityLevel);
  if (const auto* integrity = std::get_if<std::vector<BYTE>>(&integrityResult); integrity && integrity->size() >= sizeof(TOKEN_MANDATORY_LABEL)) {
    const auto* label = reinterpret_cast<const TOKEN_MANDATORY_LABEL*>(integrity->data());
    if (IsValidSid(label->Label.Sid)) {
      const DWORD count = *GetSidSubAuthorityCount(label->Label.Sid);
      if (count != 0) {
        addValue(fields, QStringLiteral("integrity.level"), integrityLevelText(*GetSidSubAuthority(label->Label.Sid, count - 1)));
      } else {
        addValue(fields, QStringLiteral("integrity.error"), QStringLiteral("TokenIntegrityLevel SID has no sub-authority"));
      }
    } else {
      addValue(fields, QStringLiteral("integrity.error"), QStringLiteral("TokenIntegrityLevel returned an invalid SID"));
    }
  } else if (const auto* error = std::get_if<DWORD>(&integrityResult)) {
    addValue(fields, QStringLiteral("integrity.error"), windowsErrorText(*error));
  } else {
    addValue(fields, QStringLiteral("integrity.error"), QStringLiteral("TokenIntegrityLevel returned no data"));
  }

  const TokenInformationResult privilegesResult = tokenInformation(token.get(), TokenPrivileges);
  const auto* privileges = std::get_if<std::vector<BYTE>>(&privilegesResult);
  constexpr std::size_t kPrivilegesHeaderSize = offsetof(TOKEN_PRIVILEGES, Privileges);
  if (!privileges || privileges->size() < kPrivilegesHeaderSize) {
    if (const auto* error = std::get_if<DWORD>(&privilegesResult))
      addValue(fields, QStringLiteral("privileges.error"), windowsErrorText(*error));
    else
      addValue(fields, QStringLiteral("privileges.error"), QStringLiteral("TokenPrivileges returned no data"));
    return fields;
  }

  const auto* tokenPrivileges = reinterpret_cast<const TOKEN_PRIVILEGES*>(privileges->data());
  const std::size_t availablePrivileges = (privileges->size() - kPrivilegesHeaderSize) / sizeof(LUID_AND_ATTRIBUTES);
  if (tokenPrivileges->PrivilegeCount > availablePrivileges) {
    addValue(fields, QStringLiteral("privileges.error"), QStringLiteral("TokenPrivileges returned malformed data"));
    return fields;
  }
  addValue(fields, QStringLiteral("privileges.count"), QString::number(tokenPrivileges->PrivilegeCount));
  for (DWORD index = 0; index < tokenPrivileges->PrivilegeCount; ++index) {
    const auto& privilege = tokenPrivileges->Privileges[index];
    QString name = privilegeName(privilege.Luid);
    if (name.isEmpty()) name = QStringLiteral("LUID-%1-%2").arg(privilege.Luid.HighPart).arg(privilege.Luid.LowPart);
    addValue(fields, QStringLiteral("privilege.%1").arg(name), privilegeState(privilege.Attributes));
  }
  return fields;
}

}  // namespace uwf::windows
