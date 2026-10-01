/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>
#include <include/ExitPointType.h>

namespace bpp::IR {

/**
 * @brief A pipeline of commands (e.g., `cmd1 | cmd2 | cmd3`)
 */
class BashPipeline : public StringType {
	public:
		ExitPointType getExitPointType() const { return exit_point_type; }
		void setExitPointType(ExitPointType type) { exit_point_type = type; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashPipeline";
			switch (getExitPointType()) {
				case ExitPointType::FUNCTION_EXIT: os << " [function exit point]"; break;
				case ExitPointType::PROGRAM_EXIT: os << " [program exit point]"; break;
				case ExitPointType::NO_EXIT: break;
				default: os << " [unknown exit point type]"; break;
			}
			os << "\n";
			StringType::prettyPrint(os, indentation_level + 1);
			os << indent << ")\n";
			return os;
		});
	private:
		ExitPointType exit_point_type = ExitPointType::NO_EXIT;
};

} // namespace bpp::IR
