/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>

namespace bpp::IR {

/**
 * @brief A Bash "break" or "continue" command, which can be part of a pipeline
 * "Break" and "continue" commands accept an optional numeric argument, which is the number of loops to break or continue. If no argument is given, the default is 1.
 * If the "break" or "continue" command is part of a pipeline with more than one element, it cannot actually exit the loop
 */
class BashBreakOrContinueCommand : public StringType {
	public:
		bool isBreak() const { return is_break; }
		void setIsBreak(bool is) { this->is_break = is; }
		bool isExitPath() const { return is_exit_path; }
		void setIsExitPath(bool is) { this->is_exit_path = is; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashBreakOrContinueCommand " << (is_break ? "break" : "continue") << "\n";
			StringType::prettyPrint(os, indentation_level + 1);
			os << indent << ")\n";
			return os;
		});
	private:
		bool is_break = false;
		/// Whether this 'break' or 'continue' can actually exit a loop: it CAN'T if it's part of a pipeline with more than one element
		bool is_exit_path = true;
};

} // namespace bpp::IR
