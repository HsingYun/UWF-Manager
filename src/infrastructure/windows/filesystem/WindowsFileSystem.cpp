// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/filesystem/WindowsFileSystem.h"

// clang-format off
#include <windows.h>
#include <objbase.h>
// clang-format on

#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "foundation/text/ObjectNameComparison.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/resources/UniqueHandle.h"

namespace uwf::winfs {
namespace {
constexpr std::size_t kTransferBufferSize = 64 * 1024;
constexpr std::int64_t kWindowsEpochOffsetMilliseconds = 11644473600000;
constexpr std::uint64_t kFileTimeTicksPerMillisecond = 10000;

bool missing(const DWORD error) { return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND; }
FileIoResult failure(const FileIoFailure stage, const DWORD error) { return {stage, {static_cast<int>(error), std::system_category()}}; }

EntryMetadata entryMetadata(const DWORD attributes, const DWORD highSize, const DWORD lowSize, const FILETIME modified) {
  const auto ticks = (static_cast<std::uint64_t>(modified.dwHighDateTime) << 32) | modified.dwLowDateTime;
  return {(attributes & FILE_ATTRIBUTE_DIRECTORY) != 0, (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0,
          (static_cast<std::uint64_t>(highSize) << 32) | lowSize,
          std::chrono::milliseconds{static_cast<std::int64_t>(ticks / kFileTimeTicksPerMillisecond) - kWindowsEpochOffsetMilliseconds}};
}

struct FindCloser {
  void operator()(const HANDLE handle) const noexcept { FindClose(handle); }
};
struct PendingFile {
  std::wstring path;
  ~PendingFile() {
    if (!path.empty()) DeleteFileW(path.c_str());
  }
};
}  // namespace

std::filesystem::path extendedPath(const std::filesystem::path& path) {
  auto native = path.native();
  if (native.empty()) throw std::invalid_argument("file-system path is empty");
  if (native.find(L'\0') != std::wstring::npos) throw std::invalid_argument("file-system path contains an embedded NUL");
  std::replace(native.begin(), native.end(), L'/', L'\\');
  if (native.starts_with(LR"(\\?\)") || native.starts_with(LR"(\\.\)")) return native;
  if (!std::filesystem::path(native).is_absolute()) {
    native = (std::filesystem::current_path() / native).native();
    std::replace(native.begin(), native.end(), L'/', L'\\');
  }
  if (native.starts_with(LR"(\\)")) return LR"(\\?\UNC\)" + native.substr(2);
  if (native.size() >= 3 && native[1] == L':' && native[2] == L'\\') return LR"(\\?\)" + native;
  throw std::invalid_argument("file-system path cannot be resolved to an absolute Windows object");
}

std::optional<EntryMetadata> metadata(const std::filesystem::path& path) {
  WIN32_FILE_ATTRIBUTE_DATA data{};
  if (GetFileAttributesExW(extendedPath(path).c_str(), GetFileExInfoStandard, &data))
    return entryMetadata(data.dwFileAttributes, data.nFileSizeHigh, data.nFileSizeLow, data.ftLastWriteTime);
  const DWORD error = GetLastError();
  if (missing(error)) return std::nullopt;
  throw windows::WindowsSystemError(error, "inspect Windows file-system object");
}

EntryType inspect(const std::filesystem::path& path) {
  const auto observed = metadata(path);
  if (!observed) return EntryType::Missing;
  if (observed->reparsePoint) return EntryType::ReparsePoint;
  return observed->directory ? EntryType::Directory : EntryType::File;
}

bool existsForVerification(const std::filesystem::path& path) { return metadata(path).has_value(); }

void enumerate(const std::filesystem::path& directory, const std::stop_token stop, const EntryVisitor& visit) {
  if (stop.stop_requested()) return;
  const auto observed = metadata(directory);
  if (!observed) throw windows::WindowsSystemError(ERROR_PATH_NOT_FOUND, "enumerate Windows directory");
  if (!observed->directory) throw windows::WindowsSystemError(ERROR_DIRECTORY, "enumerate Windows directory");
  WIN32_FIND_DATAW data{};
  const HANDLE raw = FindFirstFileW((extendedPath(directory) / L"*").c_str(), &data);
  if (raw == INVALID_HANDLE_VALUE) {
    const DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) return;  // An existing empty directory.
    throw windows::WindowsSystemError(error, "enumerate Windows directory");
  }
  const std::unique_ptr<void, FindCloser> handle(raw);
  while (!stop.stop_requested()) {
    const std::wstring_view name(data.cFileName);
    if (name != L"." && name != L"..")
      visit(directory / name, entryMetadata(data.dwFileAttributes, data.nFileSizeHigh, data.nFileSizeLow, data.ftLastWriteTime));
    if (!FindNextFileW(handle.get(), &data)) {
      const DWORD error = GetLastError();
      if (error == ERROR_NO_MORE_FILES) return;
      throw windows::WindowsSystemError(error, "enumerate Windows directory");
    }
  }
}

void createDirectory(const std::filesystem::path& path) {
  if (!CreateDirectoryW(extendedPath(path).c_str(), nullptr)) throw windows::WindowsSystemError(GetLastError(), "create Windows directory");
}

void rename(const std::filesystem::path& source, const std::filesystem::path& destination) {
  const auto nativeSource = extendedPath(source).native();
  const auto nativeDestination = extendedPath(destination).native();
  const auto sameName = compareObjectNames({reinterpret_cast<const char16_t*>(nativeSource.data()), nativeSource.size()},
                                           {reinterpret_cast<const char16_t*>(nativeDestination.data()), nativeDestination.size()}) == 0;
  if (!sameName && metadata(destination)) throw windows::WindowsSystemError(ERROR_ALREADY_EXISTS, "rename Windows object");
  if (!MoveFileExW(nativeSource.c_str(), nativeDestination.c_str(), 0)) throw windows::WindowsSystemError(GetLastError(), "rename Windows object");
}

FileIoResult readChunks(const std::filesystem::path& path, const ChunkVisitor& visit) {
  const windows::UniqueHandle file(CreateFileW(extendedPath(path).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!file.valid()) return failure(FileIoFailure::Open, GetLastError());
  std::array<std::byte, kTransferBufferSize> buffer{};
  for (;;) {
    DWORD read = 0;
    if (!ReadFile(file.get(), buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr)) return failure(FileIoFailure::Read, GetLastError());
    if (!read || !visit(std::span<const std::byte>(buffer.data(), static_cast<std::size_t>(read)))) return {};
  }
}

FileIoResult writeFile(const std::filesystem::path& path, const std::span<const std::byte> bytes, const FilePublication publication) {
  const auto target = extendedPath(path);
  const auto existing = metadata(path);
  if (publication == FilePublication::CreateNew && existing) return failure(FileIoFailure::AlreadyExists, ERROR_FILE_EXISTS);
  if (existing && existing->directory) return failure(FileIoFailure::Open, ERROR_ACCESS_DENIED);
  GUID identifier{};
  const HRESULT identifierStatus = CoCreateGuid(&identifier);
  if (FAILED(identifierStatus)) return failure(FileIoFailure::Open, static_cast<DWORD>(identifierStatus));
  std::array<wchar_t, 39> identifierText{};  // Braced GUID plus its terminator.
  if (!StringFromGUID2(identifier, identifierText.data(), static_cast<int>(identifierText.size())))
    return failure(FileIoFailure::Open, ERROR_INSUFFICIENT_BUFFER);
  auto stagingPath = (target.parent_path() / (std::wstring(L".uwf-write-") + identifierText.data() + L".tmp")).native();
  PendingFile staged;
  windows::UniqueHandle file(CreateFileW(stagingPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!file.valid()) return failure(FileIoFailure::Open, GetLastError());
  staged.path = std::move(stagingPath);
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const auto count = static_cast<DWORD>(std::min(bytes.size() - offset, kTransferBufferSize));
    DWORD written = 0;
    if (!WriteFile(file.get(), bytes.data() + offset, count, &written, nullptr)) return failure(FileIoFailure::Write, GetLastError());
    if (!written) return failure(FileIoFailure::Write, ERROR_WRITE_FAULT);
    offset += written;
  }
  if (!FlushFileBuffers(file.get())) return failure(FileIoFailure::Write, GetLastError());
  file.reset();
  if (publication == FilePublication::Replace) {
    if (ReplaceFileW(target.c_str(), staged.path.c_str(), nullptr, 0, nullptr, nullptr)) {
      staged.path.clear();
      return {};
    }
    const DWORD error = GetLastError();
    if (error != ERROR_FILE_NOT_FOUND) return failure(FileIoFailure::Write, error);
  }
  const DWORD flags = MOVEFILE_WRITE_THROUGH | (publication == FilePublication::Replace ? MOVEFILE_REPLACE_EXISTING : 0);
  if (!MoveFileExW(staged.path.c_str(), target.c_str(), flags)) {
    const DWORD error = GetLastError();
    if (publication == FilePublication::CreateNew && (error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS))
      return failure(FileIoFailure::AlreadyExists, error);
    return failure(FileIoFailure::Write, error);
  }
  staged.path.clear();
  return {};
}
}  // namespace uwf::winfs
