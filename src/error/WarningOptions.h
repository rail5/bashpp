/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <bitset>
#include <string>
#include <string_view>
#include <array>
#include <stdexcept>
#include <algorithm>
#include <optional>

// Single source of truth for all warning options in Bash++
#define BPP_WARNING_LIST(X) \
	/* WarningName,                CLI string,         enabled by default, description) */ \
	X(Bash53NativeSupershell,      "bash53-native-supershell",       true, "Warn when using Bash 5.3's native supershell syntax ${ command; } instead of Bash++'s @(command)") \
	X(CastToUnknownClass,          "cast-to-unknown-class",          true, "Warn when the target class of a @dynamic_cast is not known at compile time") \
	X(ImplicitToPrimitive,         "implicit-toprimitive",          false, "Warn when .toPrimitive is called implicitly by referencing a non-primitive object in a primitive context") \
	X(CastFromImplicitToPrimitive, "cast-from-implicit-toprimitive", true, "Warn when the input to a @dynamic_cast is an implicit call to .toPrimitive") \
	X(CastToImplicitToPrimitive,   "cast-to-implicit-toprimitive",   true, "Warn when the target class of a @dynamic_cast is an implicit call to .toPrimitive") \
	X(TypeofImplicitToPrimitive,   "typeof-implicit-toprimitive",    true, "Warn when the input to a @typeof is an implicit call to .toPrimitive")


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


constexpr std::array<std::pair<std::string_view, WarningType>, 6> warning_clistring_map = {{
	// Map CLI string names to enum values for all warnings defined in BPP_WARNING_LIST
	BPP_WARNING_LIST(BPP_WARNING_MAP_CLI_STRING_TO_ENUM)
}};

inline std::optional<WarningType> get_warning_by_cli_string(std::string_view cli_string) {
	for (const auto& [str, warning] : warning_clistring_map) {
		if (cli_string == str) return warning;
	}
	return std::nullopt;
}
inline std::optional<std::string_view> get_cli_string_by_warning(WarningType warning) {
	for (const auto& [str, warning_enum] : warning_clistring_map) {
		if (warning == warning_enum) return str;
	}
	return std::nullopt;
}

class WarningOptions {
	private:
		std::bitset<static_cast<std::size_t>(WarningType::EnumCount)> flags;
	public:
		WarningOptions() {
			// Set default values for all warnings
			BPP_WARNING_LIST(BPP_WARNING_SET_DEFAULT)
		}

		/**
		 * @brief Parse a warning option string and enable/disable the corresponding warning
		 * 
		 * @param warning_option The warning option string to parse (e.g., "bash53-native-supershell", "no-cast-to-unknown-class")
		 * @throws std::runtime_error if the warning option is unknown
		 */
		void parse(std::string_view warning_option) {
			std::string warning_cli_string(warning_option);
			// Transform to lowercase for case-insensitive comparison
			std::transform(warning_cli_string.begin(), warning_cli_string.end(),
				warning_cli_string.begin(),
				[](unsigned char c) { return std::tolower(c); });

			// Special-case: 'all' and 'none' are not valid warning names,
			// but they are valid options to enable/disable all warnings
			if (warning_cli_string == "all") { enable_all_warnings(); return; }
			if (warning_cli_string == "none") { disable_all_warnings(); return; }

			bool enable = true;

			if (warning_cli_string.starts_with("no-")) {
				warning_cli_string = warning_cli_string.substr(3);
				enable = false;
			}

			auto warning_opt = get_warning_by_cli_string(warning_cli_string);

			if (!warning_opt) throw std::runtime_error("Unknown warning option: '" + std::string(warning_option) + "'");

			if (enable) {
				enable_warning(*warning_opt);
			} else {
				disable_warning(*warning_opt);
			}
		}

		void enable_warning(WarningType warning) {
			flags.set(static_cast<std::size_t>(warning), true);
		}
		void disable_warning(WarningType warning) {
			flags.set(static_cast<std::size_t>(warning), false);
		}
		bool is_warning_enabled(WarningType warning) const {
			return flags.test(static_cast<std::size_t>(warning));
		}
		void enable_all_warnings() {
			flags.set();
		}
		void disable_all_warnings() {
			flags.reset();
		}
};

} // namespace bpp::ErrorHandling

#undef BPP_WARNING_GET_NAME
#undef BPP_WARNING_MAP_CLI_STRING_TO_ENUM
#undef BPP_WARNING_SET_DEFAULT

#undef BPP_WARNING_LIST
