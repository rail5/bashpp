/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>
#include <include/ExitPointType.h>

namespace bpp::AST {

class BashCommand : public StringType {
	private:
		ExitPointType exit_point_type = ExitPointType::NO_EXIT;
		/// Whether this is an 'exec' command specifically
		bool is_exec = false;
	public:
		constexpr BashCommand() : StringType(bpp::AST::NodeType::BashCommand) {}

		void setExitPointType(ExitPointType type) { this->exit_point_type = type; }
		ExitPointType getExitPointType() const { return this->exit_point_type; }

		void setIsExec(bool is) { this->is_exec = is; }
		bool isExec() const { return this->is_exec; }

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashCommand";
			switch (exit_point_type) {
				case ExitPointType::FUNCTION_EXIT: os << " [function exit point]"; break;
				case ExitPointType::PROGRAM_EXIT: os << " [program exit point]"; break;
				case ExitPointType::NO_EXIT: break;
				default: os << " [unknown exit point type]"; break;
			}
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
