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
	/* WarningName,                CLI string,                       enabled by default) */ \
	X(Bash53NativeSupershell,      "bash53-native-supershell",       true) \
	X(CastToUnknownClass,          "cast-to-unknown-class",          true) \
	X(ImplicitToPrimitive,         "implicit-toprimitive",          false) \
	X(CastFromImplicitToPrimitive, "cast-from-implicit-toprimitive", true) \
	X(CastToImplicitToPrimitive,   "cast-to-implicit-toprimitive",   true) \
	X(TypeofImplicitToPrimitive,   "typeof-implicit-toprimitive",    true)

namespace bpp::ErrorHandling {

class WarningOptions {
	public:
		enum class Warning : std::uint8_t {
			#define BPP_WARNING_ENUM(name, cli_string, enabled_by_default) name,
			BPP_WARNING_LIST(BPP_WARNING_ENUM)
			#undef BPP_WARNING_ENUM
			EnumCount
		};
	private:
		// Default: all warnings enabled except for ImplicitToPrimitive, which is disabled by default
		std::bitset<static_cast<std::size_t>(Warning::EnumCount)> warning_flags = []() {
			std::bitset<static_cast<std::size_t>(Warning::EnumCount)> flags;
			#define BPP_WARNING_DEFAULT(name, cli_string, enabled_by_default) \
				flags.set(static_cast<std::size_t>(Warning::name), enabled_by_default);
			BPP_WARNING_LIST(BPP_WARNING_DEFAULT)
			#undef BPP_WARNING_DEFAULT
			return flags;
		}();

		static constexpr std::array<std::pair<std::string_view, Warning>, 6> warning_name_map = {{
			#define BPP_WARNING_NAME_MAP(name, cli_string, enabled_by_default) {cli_string, Warning::name},
			BPP_WARNING_LIST(BPP_WARNING_NAME_MAP)
			#undef BPP_WARNING_NAME_MAP
		}};

		static_assert(static_cast<std::size_t>(Warning::EnumCount) <= 32, "Too many warnings defined; maximum is 32");
	public:
		static std::optional<Warning> get_warning_by_name(std::string_view warning_name) {
			for (const auto& [name, warning] : warning_name_map) {
				if (warning_name == name) return warning;
			}
			return std::nullopt;
		}
		/**
		 * @brief Parse a warning option string and enable/disable the corresponding warning
		 * 
		 * @param warning_option The warning option string to parse (e.g., "bash53-native-supershell", "no-cast-to-unknown-class")
		 * @throws std::runtime_error if the warning option is unknown
		 */
		void parse(std::string_view warning_option) {
			std::string warning_name(warning_option);
			// Transform to lowercase for case-insensitive comparison
			std::transform(warning_name.begin(), warning_name.end(),
				warning_name.begin(),
				[](unsigned char c) { return std::tolower(c); });

			// Special-case: 'all' and 'none' are not valid warning names,
			// but they are valid options to enable/disable all warnings
			if (warning_name == "all") { enable_all_warnings(); return; }
			if (warning_name == "none") { disable_all_warnings(); return; }

			bool enable = true;

			if (warning_name.starts_with("no-")) {
				warning_name = warning_name.substr(3);
				enable = false;
			}

			auto warning_opt = get_warning_by_name(warning_name);

			if (!warning_opt) throw std::runtime_error("Unknown warning option: '" + std::string(warning_option) + "'");

			enable ? enable_warning(*warning_opt) : disable_warning(*warning_opt);
		}

		void enable_warning(Warning warning) {
			warning_flags.set(static_cast<std::size_t>(warning), true);
		}
		void disable_warning(Warning warning) {
			warning_flags.set(static_cast<std::size_t>(warning), false);
		}
		bool is_warning_enabled(Warning warning) const {
			return warning_flags.test(static_cast<std::size_t>(warning));
		}
		void enable_all_warnings() {
			warning_flags.set();
		}
		void disable_all_warnings() {
			warning_flags.reset();
		}
};

} // namespace bpp::ErrorHandling

#undef BPP_WARNING_LIST
