/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/expressions/String.h>

#include <optional>
#include <vector>

namespace bpp::IR {

/**
 * @brief The "condition" of an if statement, i.e., the command sequence to be executed to determine whether the branch is taken
 * E.g., in `if true; then ...`, the BashIfCondition is `true`
 * Or, in `if [[ $x -gt 5 ]]; then ...`, the BashIfCondition is `[[ $x -gt 5 ]]`
 */
class BashIfCondition : public StringType {
	public:
		PRETTYPRINT_OVERRIDE();
};

/**
 * @brief A branch of an if statement: a condition to check, and the code to execute if that condition is true
 * If this branch is an "else" branch, it need not have a condition (although it *can* have one, as an "elif" branch)
 */
class BashIfBranch : public CodeEntity {
	private:
		bool root_branch = false;
		std::optional<std::shared_ptr<BashIfCondition>> condition = std::nullopt;
	public:
		void setIsRoot(bool is_root) { root_branch = is_root; }
		bool isRoot() const { return root_branch; }
		void setCondition(std::shared_ptr<BashIfCondition> cond) { condition = std::move(cond); }
		const std::optional<std::shared_ptr<BashIfCondition>>& getCondition() const { return condition; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

/**
 * @brief An if statement in Bash, consisting of one or more branches (if, elif, else)
 */
class BashIfStatement : public Entity {
	private:
		std::vector<std::shared_ptr<BashIfBranch>> branches;
	public:
		void addBranch(std::shared_ptr<BashIfBranch> branch) { branches.push_back(std::move(branch)); }
		const std::vector<std::shared_ptr<BashIfBranch>>& getBranches() const { return branches; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
