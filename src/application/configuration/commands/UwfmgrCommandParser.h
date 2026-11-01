// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <span>
#include <string_view>

#include "application/configuration/commands/UwfmgrCommand.h"

namespace uwf::app {

struct UwfmgrCommandDescriptor {
  UwfmgrKind kind;
  std::string_view family;
  std::string_view verb;
  std::string_view operands;
  std::string_view description;
};

// Public help capabilities only. Help aliases and protect-all share their
// command's descriptor instead of becoming additional business capabilities.
[[nodiscard]] std::span<const UwfmgrCommandDescriptor> uwfmgrCommandCatalog();

// UTF-8 Windows argv operands, already split by the process boundary. The
// executable is excluded; shell escapes and percent sequences remain literal.
// A recognized family without a verb selects that family's help.
// Session selectors are lower-case; absent selectors mean Current then Next.
// Registry commit's explicit empty value remains distinct from an absent value.
[[nodiscard]] UwfmgrCommand parseUwfmgrArguments(const std::vector<std::string>& arguments);
[[nodiscard]] bool isUwfmgrConfigurationCommand(UwfmgrKind kind);
// The root page is the public parameter overview. Family pages retain the
// captured Usage descriptions, commands, parameters and examples in English.
[[nodiscard]] std::string uwfmgrHelpText(std::string_view family = {});
[[nodiscard]] std::string uwfmgrParseErrorText(const UwfmgrCommand& command);

}  // namespace uwf::app
