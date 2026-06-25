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
 * @brief A pipeline of commands (e.g., `cmd1 | cmd2 | cmd3`)
 */
class BashPipeline : public StringType {
	public:
		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashPipeline\n";
			StringType::prettyPrint(os, indentation_level + 1);
			os << indent << ")\n";
			return os;
		});
};

} // namespace bpp::IR
