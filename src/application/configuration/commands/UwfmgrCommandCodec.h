// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string_view>

#include "application/configuration/commands/UwfmgrCommand.h"

namespace uwf::app {
inline constexpr std::string_view kUwfmgrScriptMarker = ":: UWF Manager CMD script v2 (UTF-8)";
// Declaration discovery for file loaders and the parser. Recognizes current
// and legacy headers with ASCII whitespace / a UTF-8 BOM; this does not certify
// that the complete framing or dependency guards are valid.
[[nodiscard]] bool isUwfmgrScriptMarker(std::string_view line);

// Literal command arguments cannot contain ASCII controls or DEL. The codec
// owns CMD quoting, dependency chains, UTF-8 script framing and round trips.
[[nodiscard]] bool isUwfmgrCommandRepresentable(const UwfmgrCommand& command);
// Reversible display text; executable batch files require the complete script.
[[nodiscard]] std::string renderUwfmgrCommand(const UwfmgrCommand& command);
[[nodiscard]] std::string renderUwfmgrCommandScript(const std::vector<UwfmgrCommand>& commands, std::string_view scope = {});
// V2 files place commands on separate lines, preserving dependent chains with
// validated error guards/labels. Legacy && files remain readable. Comments and
// failures retain source locations; percent decoding requires complete framing.
[[nodiscard]] std::vector<UwfmgrCommand> parseUwfmgrText(const std::string& text);
}  // namespace uwf::app
