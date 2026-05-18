/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <string>

#include "bpp.h"
#include "bpp_code_entity.h"
#include "bpp_string.h"

namespace bpp {

/**
 * @class bash_while_or_until_loop
 * 
 * @brief A while/until loop in Bash++
 * 
 * This entity gets pushed onto the entity stack when a while loop or an until loop is encountered in Bash++ code.
 * It contains a bash_while_or_until_condition object which holds the condition for the loop
 * 
 * The reason for this is that the condition for the loop may contain references which need to be resolved
 * And the pre- and post-code for those references need to be added in specific places in the compiled code.
 * E.g., supershells must be re-evaluated for each iteration of the loop
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_while_or_until_loop : public bpp_code_entity {
	private:
		std::shared_ptr<bpp::bash_while_or_until_condition> condition;
	public:
		void set_condition(std::shared_ptr<bpp::bash_while_or_until_condition> condition);
		std::shared_ptr<bpp::bash_while_or_until_condition> get_condition() const;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

/**
 * @class bash_while_or_until_condition
 * 
 * @brief The condition for a while/until loop in Bash++
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_while_or_until_condition : public bpp_string {};

} // namespace bpp
