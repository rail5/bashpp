/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/BashFunction.h>

namespace bpp::IR::Builtins {

/**
 * @brief A Bash++ system function, which is a special type of Bash function that is automatically included in every Bash++ program,
 * but whose code is only written to the output if it is actually used in the program.
 * Examples of system functions are the built-ins for supershells and dynamic casts.
 * 
 */
class SystemFunction : public BashFunction {
	private:
		std::string_view m_contents;
	public:
		SystemFunction() = delete;
		explicit SystemFunction(std::string_view contents) : m_contents(contents) {}

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR::Builtins
