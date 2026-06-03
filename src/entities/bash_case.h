/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <memory>

#include "bpp.h"
#include "bpp_string.h"
#include "bpp_code_entity.h"

namespace bpp {

/**
 * @class bash_case
 * 
 * @brief A case statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when a case statement is encountered in Bash++ code.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_case : public bpp_string {
	private:
		std::string cases;
	public:
		void add_case(const std::string& case_);

		const std::string& get_cases() const;
};

/**
 * @class bash_case_pattern
 * 
 * @brief A pattern for a case statement in Bash++
 * 
 * This entity contains a pattern to be matched in a case statement.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_case_pattern : public bpp_code_entity {
	private:
		std::string pattern;
		std::shared_ptr<bpp::bash_case> containing_case;
	public:
		void set_pattern(const std::string& pattern);
		void set_containing_case(std::shared_ptr<bpp::bash_case> containing_case);

		const std::string& get_pattern() const;
		std::shared_ptr<bpp::bash_case> get_containing_case() const;
};

} // namespace bpp
