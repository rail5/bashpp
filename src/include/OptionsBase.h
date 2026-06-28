/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <concepts>
#include <bitset>
#include <string_view>
#include <string>
#include <algorithm>
#include <optional>
#include <stdexcept>

namespace bpp {

// Concept EnumType:
// 1. Is an enum class
// 2. Has a sentinel value called EnumCount which is the last value in the enum
template <typename T>
concept EnumType = std::is_enum_v<T> && requires { T::EnumCount; };

/**
 * @brief CRTP base class for options that can be enabled or disabled.
 *
 * This is used for both warning options (-W) and optimization options (-f).
 * 
 * @tparam Derived The derived class that inherits from this base class. This is used for CRTP.
 * @tparam Enum The enum type that defines the options. This must be an enum class with a sentinel value called EnumCount.
 * @tparam string_to_enum_map A constexpr array of pairs that maps string representations of options to their corresponding enum values.
 */
template <typename Derived, EnumType Enum, const auto& string_to_enum_map>
class OptionsBase {
	private:
		Derived& self() { return static_cast<Derived&>(*this); }

		std::string error_message(const std::string& invalid_option) {
			if constexpr (requires { self().get_error_message(invalid_option); }) {
				return self().get_error_message(invalid_option);
			} else {
				return "Unknown option: '" + invalid_option + "'";
			}
		}
	protected:
		std::bitset<static_cast<std::size_t>(Enum::EnumCount)> flags;
		static constexpr auto option_map = string_to_enum_map;
	public:
		using Option = Enum;

		void enable(Option opt) { flags.set(static_cast<std::size_t>(opt), true); }
		void disable(Option opt) { flags.set(static_cast<std::size_t>(opt), false); }
		[[nodiscard]] bool is_enabled(Option opt) const { return flags.test(static_cast<std::size_t>(opt)); }

		std::optional<Option> get_option_by_cli_string(std::string_view cli_string) const {
			for (const auto& [str, option] : option_map) {
				if (cli_string == str) return option;
			}
			return std::nullopt;
		}

		std::optional<std::string_view> get_cli_string_by_option(Option opt) const {
			for (const auto& [str, option] : option_map) {
				if (opt == option) return str;
			}
			return std::nullopt;
		}

		void parse(std::string_view option) {
			std::string cli_string(option);
			// Transform to lowercase for case-insensitive comparison
			std::transform(cli_string.begin(), cli_string.end(),
				cli_string.begin(),
				[](unsigned char c) { return std::tolower(c); });

			// Special casing:
			// If the derived class defines 'enable_all' and 'disable_all' methods, then we can handle the special cases of "all" and "none" options
			if constexpr (requires { self().enable_all(); self().disable_all(); }) {
				if (cli_string == "all") { self().enable_all(); return; }
				if (cli_string == "none") { self().disable_all(); return; }
			}

			bool enable = true;
			if (cli_string.starts_with("no-")) {
				cli_string = cli_string.substr(3);
				enable = false;
			}

			auto option_opt = get_option_by_cli_string(cli_string);
			if (!option_opt) throw std::runtime_error(error_message(cli_string));

			if (enable) {
				this->enable(option_opt.value());
			} else {
				this->disable(option_opt.value());
			}
		}
};

} // namespace bpp
