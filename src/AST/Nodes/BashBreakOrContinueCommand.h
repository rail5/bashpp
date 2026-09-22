/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace bpp::AST {

class BashBreakOrContinueCommand : public StringType {
	private:
		bool is_break = false;
		/// Whether this 'break' or 'continue' can actually exit a loop: it CAN'T if it's part of a pipeline with more than one element
		bool is_exit_path = true;
	public:
		constexpr BashBreakOrContinueCommand() : StringType(bpp::AST::NodeType::BashBreakOrContinueCommand) {}

		void setIsBreak(bool is) { this->is_break = is; }
		bool isBreak() const { return this->is_break; }

		void unmarkExitPath() { this->is_exit_path = false; }
		bool isExitPath() const { return this->is_exit_path; }

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashBreakOrContinueCommand " << (is_break ? "break" : "continue");
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
