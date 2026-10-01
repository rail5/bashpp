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
 * @brief A pattern and its associated action code block in a Bash case statement.
 *
 * The pattern to be matched is stored in an associated StringType entity,
 * the action to be taken in the event that the pattern is matched is stored in the children of this entity.
 */
class BashCasePattern : public StringType {
	private:
		std::unique_ptr<StringType> pattern_header;
	public:
		void setPatternHeader(std::unique_ptr<StringType> header) { pattern_header = std::move(header); }
		const StringType* getPatternHeader() const { return pattern_header.get(); }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

/**
 * @brief A Bash case statement, consisting of a case input and zero or more case patterns.
 *
 * The case input is the value to be matched against the patterns,
 * and the patterns are the possible values to match against, each with an associated action to take if the pattern is matched.
 *
 * The input is stored in an associated StringType entity, and the patterns are stored in the children of this entity.
 */
class BashCaseStatement : public StringType {
	private:
		std::unique_ptr<StringType> case_input;
	public:
		void setCaseInput(std::unique_ptr<StringType> input) { case_input = std::move(input); }
		const StringType* getCaseInput() const { return case_input.get(); }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
