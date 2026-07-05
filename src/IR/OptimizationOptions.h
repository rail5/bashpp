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

// Single source of truth for all optimization options in Bash++
#define BPP_OPTIMIZATION_LIST(X) \
	/* optimization name,                CLI string,              optimization level, description */ \
	X(RemoveUnusedMethods,               "rm-unused-methods",                      1, "Do not write definitions for unused methods") \
	X(RemoveUnusedParameters,            "rm-unused-parameters",                   1, "Remove unused parameters from methods") \
	X(RemoveUnusedDataMembers,           "rm-unused-datamembers",                  1, "Remove unused data members from classes") \
	X(RemoveUnusedClasses,               "rm-unused-classes",                      1, "Remove unused classes") \
	X(RemoveRedundantDynamicCasts,       "rm-redundant-dynamic-casts",             1, "Replace redundant dynamic casts with their compile-time known results") \
	X(SkipTrivialDeletionsInLocalScopes, "skip-trivial-deletions-in-local-scopes", 1, "Skip deleting objects with trivial destructors when they are in local scopes") \
	\
	X(SkipThisPtrValidation,             "skip-thisptr-validation",                2, "Skip validation of @this pointer in methods when it is not needed") \

#define BPP_OPTIMIZATION_GET_NAME(name, cli_string, level, description) \
	name,

#define BPP_OPTIMIZATION_MAP_CLI_STRING_TO_ENUM(name, cli_string, level, description) \
	{cli_string, OptimizationType::name},

#define BPP_DISABLE_OPTIMIZATIONS(name, cli_string, level, description) \
	flags.set(static_cast<std::size_t>(OptimizationType::name), false);

#define BPP_OPTIMIZATION_SET_LEVEL1(name, cli_string, level, description) \
	if constexpr (level == 1) flags.set(static_cast<std::size_t>(OptimizationType::name), true);

#define BPP_OPTIMIZATION_SET_LEVEL2(name, cli_string, level, description) \
	if constexpr (level == 2) flags.set(static_cast<std::size_t>(OptimizationType::name), true);

namespace bpp::IR {

enum class OptimizationType : std::uint8_t {
	// List all optimizations defined in BPP_OPTIMIZATION_LIST as enum values
	BPP_OPTIMIZATION_LIST(BPP_OPTIMIZATION_GET_NAME)
	EnumCount // Sentinel value to indicate the number of optimizations defined
};

constexpr std::array<std::pair<std::string_view, OptimizationType>, 9> optimization_clistring_map = {{
	// Map CLI string names to enum values for all optimizations defined in BPP_OPTIMIZATION_LIST
	BPP_OPTIMIZATION_LIST(BPP_OPTIMIZATION_MAP_CLI_STRING_TO_ENUM)
}};

class OptimizationOptions : public OptionsBase<OptimizationOptions, OptimizationType, optimization_clistring_map> {
	public:
		OptimizationOptions() {
			set_optimization_level(1);
		}

		static std::string get_error_message(const std::string& invalid_option) {
			return "Unknown optimization flag: '" + invalid_option + "'";
		}

		void set_optimization_level(std::uint8_t level) {
			switch (level) {
				default:
					[[fallthrough]];
				case 2:
					BPP_OPTIMIZATION_LIST(BPP_OPTIMIZATION_SET_LEVEL2)
					[[fallthrough]];
				case 1:
					BPP_OPTIMIZATION_LIST(BPP_OPTIMIZATION_SET_LEVEL1)
					break;
				case 0:
					BPP_OPTIMIZATION_LIST(BPP_DISABLE_OPTIMIZATIONS)
					break;
			}
		}
};

} // namespace bpp::IR

#undef BPP_OPTIMIZATION_MAP_CLI_STRING_TO_ENUM
#undef BPP_OPTIMIZATION_GET_NAME
#undef BPP_OPTIMIZATION_LIST
