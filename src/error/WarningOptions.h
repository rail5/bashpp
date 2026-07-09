/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <include/OptionsBase.h>

// Single source of truth for all warning options in Bash++
#define BPP_WARNING_LIST(X) \
	/* warning name,               CLI string,         enabled by default, description */ \
	X(Bash53NativeSupershell,      "bash53-native-supershell",       true, "Warn when using Bash 5.3's native supershell syntax ${ command; } instead of Bash++'s @(command)") \
	X(CastToUnknownClass,          "cast-to-unknown-class",          true, "Warn when the target class of a @dynamic_cast is not known at compile time") \
	X(ImplicitToPrimitive,         "implicit-toprimitive",          false, "Warn when .toPrimitive is called implicitly by referencing a non-primitive object in a primitive context") \
	X(CastFromImplicitToPrimitive, "cast-from-implicit-toprimitive", true, "Warn when the input to a @dynamic_cast is an implicit call to .toPrimitive") \
	X(CastToImplicitToPrimitive,   "cast-to-implicit-toprimitive",   true, "Warn when the target class of a @dynamic_cast is an implicit call to .toPrimitive") \
	X(TypeofImplicitToPrimitive,   "typeof-implicit-toprimitive",    true, "Warn when the input to a @typeof is an implicit call to .toPrimitive") \
	X(DubiousFunctionName,         "dubious-function-name",          true, "Warn when a function (not method) name may conflict with Bash++ built-in functions or generated symbols")


#define BPP_WARNING_GET_NAME(name, cli_string, enabled_by_default, description) \
	name,

#define BPP_WARNING_MAP_CLI_STRING_TO_ENUM(name, cli_string, enabled_by_default, description) \
	{cli_string, WarningType::name},

#define BPP_WARNING_SET_DEFAULT(name, cli_string, enabled_by_default, description) \
	flags.set(static_cast<std::size_t>(WarningType::name), enabled_by_default);

namespace bpp::ErrorHandling {

enum class WarningType : std::uint8_t {
	// List all warnings defined in BPP_WARNING_LIST as enum values
	BPP_WARNING_LIST(BPP_WARNING_GET_NAME)
	EnumCount // Sentinel value to indicate the number of warnings defined
};

constexpr std::size_t warning_count = static_cast<std::size_t>(WarningType::EnumCount);

constexpr std::array<std::pair<std::string_view, WarningType>, warning_count> warning_clistring_map = {{
	// Map CLI string names to enum values for all warnings defined in BPP_WARNING_LIST
	BPP_WARNING_LIST(BPP_WARNING_MAP_CLI_STRING_TO_ENUM)
}};

class WarningOptions : public OptionsBase<WarningOptions, WarningType, warning_clistring_map> {
	public:
		WarningOptions() {
			BPP_WARNING_LIST(BPP_WARNING_SET_DEFAULT)
		}

		static std::string get_error_message(const std::string& invalid_option) {
			return "Unknown warning flag: '" + invalid_option + "'";
		}

		void enable_all() { flags.set(); }
		void disable_all() { flags.reset(); }
};

} // namespace bpp::ErrorHandling

#undef BPP_WARNING_GET_NAME
#undef BPP_WARNING_MAP_CLI_STRING_TO_ENUM
#undef BPP_WARNING_SET_DEFAULT

#undef BPP_WARNING_LIST
