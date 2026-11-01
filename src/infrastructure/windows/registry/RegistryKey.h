// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Registry paths accept short or full hive names and preserve the key suffix
// literally. Names must be valid UTF-8 without embedded NUL. Reads and value
// writes explicitly select the 64-bit view.

#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "application/registry/contracts/RegistryValueInfo.h"
#include "foundation/paths/RegistryPath.h"

namespace uwf::regkey {

// Only an explicit not-found result returns false; other failures throw.
[[nodiscard]] bool keyExists(std::string_view key);

// An empty valueName selects the default value. Only a missing key or value
// returns false; other failures throw.
[[nodiscard]] bool valueExists(std::string_view key, std::string_view valueName);

// Reads REG_SZ/REG_EXPAND_SZ without environment expansion. Missing values
// return nullopt; empty strings remain present. Type and read failures throw.
[[nodiscard]] std::optional<std::string> readString(std::string_view key, std::string_view valueName);

// Missing values return nullopt; zero remains present. Type, size and read
// failures throw.
[[nodiscard]] std::optional<std::uint32_t> readDword(std::string_view key, std::string_view valueName);

// Missing values return nullopt; an empty list remains present. Invalid type,
// malformed termination and read failures throw.
[[nodiscard]] std::optional<std::vector<std::string>> readMultiString(std::string_view key, std::string_view valueName);

// Validates and serializes every entry before creating the key or replacing
// its REG_MULTI_SZ value. Entries must be nonempty UTF-8 without embedded NUL.
void writeMultiString(std::string_view key, std::string_view valueName, const std::vector<std::string>& values);

// Deletes the subtree in the process's native registry view. Missing keys are
// an idempotent success; hive roots are rejected and other failures throw.
void deleteTree(std::string_view key);

// Returns direct child names. Missing keys return an empty list; failures throw.
[[nodiscard]] std::vector<std::string> subkeyNames(std::string_view key, std::stop_token stop = {});
void visitSubkeyNames(std::string_view key, const std::function<void(const std::string&)>& discover, std::stop_token stop = {});

// Queries the child count without enumeration. Missing keys return false.
[[nodiscard]] bool hasSubkeys(std::string_view key);

// Includes the default value under its empty name. Missing keys return an
// empty list; enumeration failures throw.
[[nodiscard]] std::vector<std::string> valueNames(std::string_view key, std::stop_token stop = {});
void visitValueNames(std::string_view key, const std::function<void(const std::string&)>& discover, std::stop_token stop = {});

// Preview data is bounded; oversized values retain their name and type.
[[nodiscard]] std::vector<app::RegistryValueInfo> values(std::string_view key, std::stop_token stop = {});

[[nodiscard]] std::vector<std::string> rootHiveLongNames();

// Returns full paths in postorder, including key last, for child-first deletion.
// Hive names are expanded; key suffixes remain literal.
[[nodiscard]] std::vector<std::string> collectKeyTree(const std::string& key);

}  // namespace uwf::regkey
