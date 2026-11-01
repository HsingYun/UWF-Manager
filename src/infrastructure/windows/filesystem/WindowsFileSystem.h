// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <stop_token>
#include <system_error>

namespace uwf::winfs {
[[nodiscard]] std::filesystem::path extendedPath(const std::filesystem::path& path);

struct EntryMetadata {
  bool directory = false;
  bool reparsePoint = false;
  std::uint64_t size = 0;
  std::chrono::milliseconds modifiedSinceUnixEpoch{};
};

// Every operation addresses the literal Windows object, including components
// ending in a space or dot. Missing identity is distinct from a failed read.
[[nodiscard]] std::optional<EntryMetadata> metadata(const std::filesystem::path& path);
using EntryVisitor = std::function<void(const std::filesystem::path&, const EntryMetadata&)>;
void enumerate(const std::filesystem::path& directory, std::stop_token stop, const EntryVisitor& visit);
void createDirectory(const std::filesystem::path& path);
void rename(const std::filesystem::path& source, const std::filesystem::path& destination);

enum class FileIoFailure { None, Open, Read, Write, AlreadyExists };
struct FileIoResult {
  FileIoFailure failure = FileIoFailure::None;
  std::error_code error;
};
using ChunkVisitor = std::function<bool(std::span<const std::byte>)>;
// The visitor borrows a chunk only during its callback. Returning false stops
// after that chunk; collecting a bounded observation is not an I/O error.
[[nodiscard]] FileIoResult readChunks(const std::filesystem::path& path, const ChunkVisitor& visit);
enum class FilePublication { Replace, CreateNew };
// Stage in the target's directory, flush, and publish atomically. CreateNew
// never replaces an existing target; Replace preserves its security metadata.
[[nodiscard]] FileIoResult writeFile(const std::filesystem::path& path, std::span<const std::byte> bytes, FilePublication publication);

// Reparse points take precedence over file/directory classification so callers
// can establish traversal boundaries before following them.
enum class EntryType {
  Missing,
  File,
  Directory,
  ReparsePoint,
};

// Only explicit path absence returns Missing; invalid paths and query failures throw.
[[nodiscard]] EntryType inspect(const std::filesystem::path& path);

[[nodiscard]] bool existsForVerification(const std::filesystem::path& path);

}  // namespace uwf::winfs
