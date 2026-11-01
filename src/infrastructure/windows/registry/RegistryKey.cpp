// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/registry/RegistryKey.h"

#include <windows.h>

#include <algorithm>
#include <format>
#include <limits>
#include <stdexcept>
#include <utility>

#include "foundation/text/StringUtil.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/registry/RegistryLimits.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf::regkey {

namespace {

// Predefined HKEY macros are not constant expressions.
struct Hive {
  const char* longForm;
  HKEY handle;
};
const Hive kHives[] = {
    {"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},   {"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
    {"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},     {"HKEY_USERS", HKEY_USERS},
    {"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG},
};

class RegistryHandle final {
 public:
  RegistryHandle() = default;
  ~RegistryHandle() {
    if (m_handle) RegCloseKey(m_handle);
  }
  RegistryHandle(const RegistryHandle&) = delete;
  RegistryHandle& operator=(const RegistryHandle&) = delete;

  [[nodiscard]] HKEY get() const noexcept { return m_handle; }
  [[nodiscard]] HKEY* put() noexcept {
    if (m_handle) RegCloseKey(m_handle);
    m_handle = nullptr;
    return &m_handle;
  }

 private:
  HKEY m_handle = nullptr;
};

struct RegistryAddress {
  HKEY root = nullptr;
  std::wstring subkey;
};

std::wstring registryName(const std::string_view name) {
  if (name.find('\0') != std::string_view::npos) throw windows::WindowsSystemError(ERROR_INVALID_PARAMETER, "registry object name contains an embedded NUL");
  return windows::utf8ToWide(name);
}

LONG resolveAddress(std::string_view key, RegistryAddress& address) {
  if (key.find('\0') != std::string_view::npos) return ERROR_INVALID_PARAMETER;
  const std::string expanded = expandHive(std::string(key));
  if (expanded.empty()) return ERROR_INVALID_PARAMETER;
  const size_t firstSlash = expanded.find('\\');

  const std::string hive = toUpperAscii(firstSlash == std::string::npos ? expanded : expanded.substr(0, firstSlash));
  for (const auto& candidate : kHives) {
    if (hive == candidate.longForm) {
      address.root = candidate.handle;
      break;
    }
  }
  if (!address.root) return ERROR_INVALID_PARAMETER;

  address.subkey = firstSlash == std::string::npos ? std::wstring{} : registryName(expanded.substr(firstSlash + 1));
  return ERROR_SUCCESS;
}

// An empty subkey opens the hive itself.
LONG openForReadStatus(std::string_view key, RegistryHandle& outKey) {
  RegistryAddress address;
  const LONG resolveStatus = resolveAddress(key, address);
  if (resolveStatus != ERROR_SUCCESS) return resolveStatus;
  return RegOpenKeyExW(address.root, address.subkey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, outKey.put());
}

bool isNotFound(const LONG status) { return status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND; }

[[noreturn]] void throwRegistryProbeError(const LONG status, const std::string_view operation) {
  throw windows::WindowsSystemError(static_cast<std::uint32_t>(status), operation);
}

[[noreturn]] void throwRegistryTypeError(const DWORD actualType, const std::string_view expectedType) {
  throw windows::WindowsSystemError(ERROR_DATATYPE_MISMATCH, std::format("read registry value: expected {}, got type {}", expectedType, actualType));
}

bool openForRead(std::string_view key, RegistryHandle& outKey) {
  const LONG status = openForReadStatus(key, outKey);
  if (isNotFound(status)) return false;
  if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "open registry key for reading");
  return true;
}

}  // namespace

bool keyExists(std::string_view key) {
  RegistryHandle opened;
  const LONG status = openForReadStatus(key, opened);
  if (isNotFound(status)) return false;
  if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "probe registry key existence");
  return true;
}

bool valueExists(std::string_view key, std::string_view valueName) {
  const std::wstring wideValue = registryName(valueName);
  RegistryHandle opened;
  const LONG openStatus = openForReadStatus(key, opened);
  if (isNotFound(openStatus)) return false;
  if (openStatus != ERROR_SUCCESS) throwRegistryProbeError(openStatus, "probe registry value existence");
  const LONG queryStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, nullptr, nullptr, nullptr);
  if (isNotFound(queryStatus)) return false;
  if (queryStatus != ERROR_SUCCESS) throwRegistryProbeError(queryStatus, "probe registry value existence");
  return true;
}

std::optional<std::string> readString(std::string_view key, std::string_view valueName) {
  const std::wstring wideValue = registryName(valueName);
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  DWORD type = 0;
  DWORD bytes = 0;
  const LONG sizeStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &type, nullptr, &bytes);
  if (isNotFound(sizeStatus)) return std::nullopt;
  if (sizeStatus != ERROR_SUCCESS) throwRegistryProbeError(sizeStatus, "read registry string size");
  if (type != REG_SZ && type != REG_EXPAND_SZ) throwRegistryTypeError(type, "REG_SZ or REG_EXPAND_SZ");
  if (bytes == 0) return std::string{};
  if (bytes % sizeof(wchar_t) != 0) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry string with an invalid byte length");

  // Registry strings are not required to include a NUL terminator.
  std::wstring buf(bytes / sizeof(wchar_t) + 1, L'\0');
  DWORD got = static_cast<DWORD>(buf.size() * sizeof(wchar_t));
  DWORD readType = 0;
  const LONG readStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &readType, reinterpret_cast<LPBYTE>(buf.data()), &got);
  if (isNotFound(readStatus)) return std::nullopt;
  if (readStatus != ERROR_SUCCESS) throwRegistryProbeError(readStatus, "read registry string");
  if (readType != REG_SZ && readType != REG_EXPAND_SZ) throwRegistryTypeError(readType, "REG_SZ or REG_EXPAND_SZ");
  if (got % sizeof(wchar_t) != 0) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry string with an invalid byte length");

  std::size_t characters = got / sizeof(wchar_t);
  while (characters > 0 && buf[characters - 1] == L'\0') --characters;
  return windows::wideToUtf8(std::wstring_view(buf.data(), characters));
}

std::optional<std::uint32_t> readDword(std::string_view key, std::string_view valueName) {
  const std::wstring wideValue = registryName(valueName);
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  DWORD value = 0;
  DWORD type = 0;
  DWORD bytes = sizeof(value);
  const LONG status = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(&value), &bytes);
  if (isNotFound(status)) return std::nullopt;
  if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "read registry DWORD");
  if (type != REG_DWORD) throwRegistryTypeError(type, "REG_DWORD");
  if (bytes != sizeof(value)) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry DWORD with an invalid byte length");
  return value;
}

std::optional<std::vector<std::string>> readMultiString(std::string_view key, std::string_view valueName) {
  const std::wstring wideValue = registryName(valueName);
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  DWORD type = 0;
  DWORD bytes = 0;
  const LONG sizeStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &type, nullptr, &bytes);
  if (isNotFound(sizeStatus)) return std::nullopt;
  if (sizeStatus != ERROR_SUCCESS) throwRegistryProbeError(sizeStatus, "read registry multi-string size");
  if (type != REG_MULTI_SZ) throwRegistryTypeError(type, "REG_MULTI_SZ");
  if (bytes < 2 * sizeof(wchar_t) || bytes % sizeof(wchar_t) != 0) {
    throwRegistryProbeError(ERROR_INVALID_DATA, "read registry multi-string with an invalid byte length");
  }

  std::vector<wchar_t> data(bytes / sizeof(wchar_t), L'\0');
  DWORD got = bytes;
  DWORD readType = 0;
  const LONG readStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &readType, reinterpret_cast<LPBYTE>(data.data()), &got);
  if (isNotFound(readStatus)) return std::nullopt;
  if (readStatus != ERROR_SUCCESS) throwRegistryProbeError(readStatus, "read registry multi-string");
  if (readType != REG_MULTI_SZ) throwRegistryTypeError(readType, "REG_MULTI_SZ");
  if (got < 2 * sizeof(wchar_t) || got % sizeof(wchar_t) != 0) {
    throwRegistryProbeError(ERROR_INVALID_DATA, "read registry multi-string with an invalid byte length");
  }

  const std::size_t characters = got / sizeof(wchar_t);
  if (data[characters - 1] != L'\0' || data[characters - 2] != L'\0') {
    throwRegistryProbeError(ERROR_INVALID_DATA, "read registry multi-string without a double-NUL terminator");
  }

  std::vector<std::string> values;
  std::size_t offset = 0;
  while (offset + 1 < characters && data[offset] != L'\0') {
    const auto begin = data.cbegin() + static_cast<std::ptrdiff_t>(offset);
    const auto end = std::find(begin, data.cend(), L'\0');
    if (end == data.cend()) {
      throwRegistryProbeError(ERROR_INVALID_DATA, "read malformed registry multi-string");
    }
    values.push_back(windows::wideToUtf8(std::wstring_view(data.data() + offset, static_cast<std::size_t>(end - begin))));
    offset = static_cast<std::size_t>(end - data.cbegin()) + 1;
  }
  if (std::any_of(data.cbegin() + static_cast<std::ptrdiff_t>(offset), data.cend(), [](const wchar_t value) { return value != L'\0'; })) {
    throwRegistryProbeError(ERROR_INVALID_DATA, "read registry multi-string with data after its terminator");
  }
  return values;
}

void writeMultiString(std::string_view key, std::string_view valueName, const std::vector<std::string>& values) {
  RegistryAddress address;
  const LONG resolveStatus = resolveAddress(key, address);
  if (resolveStatus != ERROR_SUCCESS) throwRegistryProbeError(resolveStatus, "resolve registry key for writing");

  // Complete validation and serialization before the first mutating Win32 call.
  const std::wstring wideValue = registryName(valueName);
  std::vector<wchar_t> data;
  for (const auto& value : values) {
    if (value.empty()) throw std::invalid_argument("registry multi-string entries cannot be empty");
    if (value.find('\0') != std::string::npos) {
      throw std::invalid_argument("registry multi-string entries cannot contain NUL characters");
    }
    const std::wstring wide = windows::utf8ToWide(value);
    data.insert(data.end(), wide.begin(), wide.end());
    data.push_back(L'\0');
  }
  data.push_back(L'\0');
  if (values.empty()) data.push_back(L'\0');

  if (data.size() > std::numeric_limits<DWORD>::max() / sizeof(wchar_t)) {
    throw std::length_error("registry multi-string is too large");
  }
  const auto bytes = static_cast<DWORD>(data.size() * sizeof(wchar_t));
  RegistryHandle opened;
  const LONG createStatus = RegCreateKeyExW(address.root, address.subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr,
                                            opened.put(), nullptr);
  if (createStatus != ERROR_SUCCESS) throwRegistryProbeError(createStatus, "create registry key for writing");
  const LONG setStatus = RegSetValueExW(opened.get(), wideValue.c_str(), 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(data.data()), bytes);
  if (setStatus != ERROR_SUCCESS) throwRegistryProbeError(setStatus, "write registry multi-string");
}

void deleteTree(std::string_view key) {
  RegistryAddress address;
  const LONG resolveStatus = resolveAddress(key, address);
  if (resolveStatus != ERROR_SUCCESS) throwRegistryProbeError(resolveStatus, "resolve registry key for recursive deletion");
  if (address.subkey.empty()) throw std::invalid_argument("a registry hive root cannot be recursively deleted");

  const LONG deleteStatus = RegDeleteTreeW(address.root, address.subkey.c_str());
  if (isNotFound(deleteStatus)) return;
  if (deleteStatus != ERROR_SUCCESS) throwRegistryProbeError(deleteStatus, "recursively delete registry key");
}

std::vector<std::string> subkeyNames(std::string_view key, const std::stop_token stop) {
  std::vector<std::string> out;
  visitSubkeyNames(key, [&](const std::string& name) { out.push_back(name); }, stop);
  return out;
}
void visitSubkeyNames(std::string_view key, const std::function<void(const std::string&)>& discover, const std::stop_token stop) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return;
  wchar_t name[windows::kRegistryKeyNameBufChars];
  for (DWORD i = 0;; ++i) {
    if (stop.stop_requested()) break;
    DWORD len = windows::kRegistryKeyNameBufChars;
    const LONG status = RegEnumKeyExW(opened.get(), i, name, &len, nullptr, nullptr, nullptr, nullptr);
    if (status == ERROR_NO_MORE_ITEMS) break;
    if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "enumerate registry subkeys");
    discover(windows::wideToUtf8(std::wstring(name, len)));
  }
}

bool hasSubkeys(std::string_view key) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return false;
  DWORD subkeyCount = 0;
  const LONG rc = RegQueryInfoKeyW(opened.get(), nullptr, nullptr, nullptr, &subkeyCount, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  if (rc != ERROR_SUCCESS) throwRegistryProbeError(rc, "read registry subkey count");
  return subkeyCount > 0;
}

std::vector<std::string> valueNames(std::string_view key, const std::stop_token stop) {
  std::vector<std::string> out;
  visitValueNames(key, [&](const std::string& name) { out.push_back(name); }, stop);
  return out;
}
void visitValueNames(std::string_view key, const std::function<void(const std::string&)>& discover, const std::stop_token stop) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return;
  std::wstring name(windows::kRegistryValueNameBufChars, L'\0');
  for (DWORD i = 0;; ++i) {
    if (stop.stop_requested()) break;
    DWORD len = static_cast<DWORD>(name.size());
    const LONG status = RegEnumValueW(opened.get(), i, name.data(), &len, nullptr, nullptr, nullptr, nullptr);
    if (status == ERROR_NO_MORE_ITEMS) break;
    if (status != ERROR_SUCCESS) throwRegistryProbeError(status, "enumerate registry value names");
    discover(windows::wideToUtf8(std::wstring(name.data(), len)));
  }
}

std::vector<app::RegistryValueInfo> values(std::string_view key, const std::stop_token stop) {
  constexpr DWORD kMaxPreviewValueBytes = 16 * 1024 * 1024;
  std::vector<app::RegistryValueInfo> out;
  RegistryHandle opened;
  if (stop.stop_requested() || !openForRead(key, opened)) return out;
  std::wstring name(windows::kRegistryValueNameBufChars, L'\0');
  // Retry after growth; data from an ERROR_MORE_DATA call may be incomplete.
  std::vector<uint8_t> data(4096);
  for (DWORD i = 0;; ++i) {
    if (stop.stop_requested()) break;
    DWORD nameLen = static_cast<DWORD>(name.size());
    DWORD dataLen = static_cast<DWORD>(data.size());
    DWORD type = 0;
    LONG rc = RegEnumValueW(opened.get(), i, name.data(), &nameLen, nullptr, &type, data.data(), &dataLen);
    while (rc == ERROR_MORE_DATA) {
      if (stop.stop_requested()) return out;
      // Bound preview allocation while retaining the returned name and type.
      if (dataLen > kMaxPreviewValueBytes) {
        out.push_back({windows::wideToUtf8(std::wstring(name.data(), nameLen)), type, std::nullopt});
        break;
      }
      if (dataLen <= data.size()) throwRegistryProbeError(rc, "enumerate registry values");
      data.resize(dataLen);
      nameLen = static_cast<DWORD>(name.size());
      dataLen = static_cast<DWORD>(data.size());
      rc = RegEnumValueW(opened.get(), i, name.data(), &nameLen, nullptr, &type, data.data(), &dataLen);
    }
    if (rc == ERROR_MORE_DATA) continue;  // Metadata was retained without preview data.
    if (rc == ERROR_NO_MORE_ITEMS) break;
    if (rc != ERROR_SUCCESS) throwRegistryProbeError(rc, "enumerate registry values");
    out.push_back({windows::wideToUtf8(std::wstring(name.data(), nameLen)), type, std::vector<uint8_t>(data.begin(), data.begin() + dataLen)});
  }
  return out;
}

std::vector<std::string> rootHiveLongNames() {
  std::vector<std::string> out;
  out.reserve(std::size(kHives));
  for (const auto& h : kHives) out.emplace_back(h.longForm);
  return out;
}

std::vector<std::string> collectKeyTree(const std::string& key) {
  std::vector<std::string> out;
  const std::string expanded = expandHive(key);
  if (!keyExists(expanded)) return out;
  for (const auto& child : subkeyNames(expanded)) {
    for (auto& descendant : collectKeyTree(expanded + '\\' + child)) out.push_back(std::move(descendant));
  }
  out.push_back(expanded);
  return out;
}

}  // namespace uwf::regkey
