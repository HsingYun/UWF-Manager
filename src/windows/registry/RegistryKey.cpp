// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "app/ports/RegistryKey.h"

#include <windows.h>

#include <algorithm>
#include <format>
#include <limits>
#include <stdexcept>
#include <utility>

#include "shared/StringUtil.h"
#include "windows/registry/RegistryLimits.h"
#include "windows/system/UnicodeConversion.h"
#include "windows/system/WindowsSystemError.h"

namespace uwf::regkey {

namespace {

// 同一映射表用于 hive 归一化和句柄查找；预定义 HKEY 宏不是常量表达式。
struct Hive {
  const char* shortForm;
  const char* longForm;
  HKEY handle;
};
const Hive kHives[] = {
    {"HKLM", "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},   {"HKCU", "HKEY_CURRENT_USER", HKEY_CURRENT_USER},
    {"HKCR", "HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},     {"HKU", "HKEY_USERS", HKEY_USERS},
    {"HKCC", "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG},
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

LONG resolveAddress(std::string_view key, RegistryAddress& address) {
  const std::string normalized = normalize(std::string(key));
  if (normalized.empty()) return ERROR_INVALID_PARAMETER;
  const size_t firstSlash = normalized.find('\\');

  const std::string hive = toUpperAscii(firstSlash == std::string::npos ? normalized : normalized.substr(0, firstSlash));
  for (const auto& candidate : kHives) {
    if (hive == candidate.longForm) {
      address.root = candidate.handle;
      break;
    }
  }
  if (!address.root) return ERROR_INVALID_PARAMETER;

  address.subkey = firstSlash == std::string::npos ? std::wstring{} : windows::utf8ToWide(normalized.substr(firstSlash + 1));
  return ERROR_SUCCESS;
}

// 归一化后只读打开键；返回的句柄由调用方关闭。根 hive 允许使用空子键打开。
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
  RegistryHandle opened;
  const LONG openStatus = openForReadStatus(key, opened);
  if (isNotFound(openStatus)) return false;
  if (openStatus != ERROR_SUCCESS) throwRegistryProbeError(openStatus, "probe registry value existence");
  // 空值名按 Win32 约定读取默认值。
  const std::wstring wideValue = windows::utf8ToWide(valueName);
  const LONG queryStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, nullptr, nullptr, nullptr);
  if (isNotFound(queryStatus)) return false;
  if (queryStatus != ERROR_SUCCESS) throwRegistryProbeError(queryStatus, "probe registry value existence");
  return true;
}

std::optional<std::string> readString(std::string_view key, std::string_view valueName) {
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  const std::wstring wideValue = windows::utf8ToWide(valueName);
  DWORD type = 0;
  DWORD bytes = 0;
  // 先量长度并确认类型。缺失是正常的可选状态；存在但类型不符是数据契约错误。
  const LONG sizeStatus = RegQueryValueExW(opened.get(), wideValue.c_str(), nullptr, &type, nullptr, &bytes);
  if (isNotFound(sizeStatus)) return std::nullopt;
  if (sizeStatus != ERROR_SUCCESS) throwRegistryProbeError(sizeStatus, "read registry string size");
  if (type != REG_SZ && type != REG_EXPAND_SZ) throwRegistryTypeError(type, "REG_SZ or REG_EXPAND_SZ");
  if (bytes == 0) return std::string{};
  if (bytes % sizeof(wchar_t) != 0) throwRegistryProbeError(ERROR_INVALID_DATA, "read registry string with an invalid byte length");

  // 多留 1 个 wchar_t：即使注册表数据未以 NUL 结尾，缓冲区也必有 NUL 收尾。
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
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  const std::wstring wideValue = windows::utf8ToWide(valueName);
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
  RegistryHandle opened;
  if (!openForRead(key, opened)) return std::nullopt;

  const std::wstring wideValue = windows::utf8ToWide(valueName);
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

  RegistryHandle opened;
  DWORD disposition = 0;
  const LONG createStatus = RegCreateKeyExW(address.root, address.subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr,
                                            opened.put(), &disposition);
  if (createStatus != ERROR_SUCCESS) throwRegistryProbeError(createStatus, "create registry key for writing");

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

  const std::wstring wideValue = windows::utf8ToWide(valueName);
  if (data.size() > std::numeric_limits<DWORD>::max() / sizeof(wchar_t)) {
    throw std::length_error("registry multi-string is too large");
  }
  const auto bytes = static_cast<DWORD>(data.size() * sizeof(wchar_t));
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

std::vector<app::RegistryValueInfo> values(std::string_view key) {
  constexpr DWORD kMaxPreviewValueBytes = 16 * 1024 * 1024;
  std::vector<app::RegistryValueInfo> out;
  RegistryHandle opened;
  if (!openForRead(key, opened)) return out;
  std::wstring name(windows::kRegistryValueNameBufChars, L'\0');
  // ERROR_MORE_DATA 后扩容并重新读取，不使用失败调用可能留下的部分数据。
  std::vector<uint8_t> data(4096);
  for (DWORD i = 0;; ++i) {
    DWORD nameLen = static_cast<DWORD>(name.size());
    DWORD dataLen = static_cast<DWORD>(data.size());
    DWORD type = 0;
    LONG rc = RegEnumValueW(opened.get(), i, name.data(), &nameLen, nullptr, &type, data.data(), &dataLen);
    while (rc == ERROR_MORE_DATA) {
      // Picker 只展示最多 200 字符的预览，不应为异常或恶意注册表值分配任意
      // 大小的内存。名称和类型已经由 RegEnumValueW 返回，超限时保留这一行，
      // 只把预览标为不可用；不能因此丢掉该值或让整张表失败。
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
    if (rc == ERROR_MORE_DATA) continue;  // 超过安全预览上限，元信息已保留
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
  const std::string norm = normalize(key);
  if (!keyExists(norm)) return out;
  // 后序 DFS：先递归收子键、再追加自身——子键天然排在父键之前。
  for (const auto& child : subkeyNames(norm)) {
    for (auto& descendant : collectKeyTree(norm + '\\' + child)) out.push_back(std::move(descendant));
  }
  out.push_back(norm);
  return out;
}

}  // namespace uwf::regkey
