/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <vector>
#include <utility>
#include <string>
#include <memory>

#include "bpp.h"
#include "bpp_string.h"
#include "bpp_code_entity.h"

namespace bpp {

/**
 * @class bash_if
 * 
 * @brief An if statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when an if statement is encountered in Bash++ code.
 * It contains a vector of conditional branches, each of which contains a condition and a branch of code
 * 
 * The reason this requires its own entity type is similar to the reason for bash_while_or_until_loop:
 * The conditions for the if statement may contain references which need to be resolved,
 * And the pre- and post-code for those references need to be added in specific places in the compiled code.
 * 
 * In the case of 'if' statements, the pre- and post-code is added before and after the entire if statement.
 * If these were parsed without their own entity type (e.g., just using a bpp_code_entity), the pre- and post-code
 * would be added before and after each individual conditional branch, which is incorrect.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_if : public bpp_string {
	private:
		std::vector<std::pair<std::string, std::string>> conditional_branches;
	public:
		void new_branch();
		void add_condition_code(const std::string& condition_code);
		void add_branch_code(const std::string& branch_code);
		const std::vector<std::pair<std::string, std::string>>& get_conditional_branches() const;
};

/**
 * @class bash_if_branch
 * 
 * @brief A branch of an if statement in Bash++
 * 
 * This entity contains the *code* which is executed for a given branch of an if statement.
 * It is not responsible for parsing the condition of the if statement.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_if_branch : public bpp_code_entity {
	private:
		std::shared_ptr<bpp::bash_if> if_statement;
	public:
		void set_if_statement(std::shared_ptr<bpp::bash_if> if_statement);
		std::shared_ptr<bpp::bash_if> get_if_statement() const;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

} // namespace bpp
