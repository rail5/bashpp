/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>

namespace bpp::AST {
enum class NodeType : std::uint8_t {
	Program = 0,
	ArrayAssignment,
	ArrayIndex,
	BashArithmeticForCondition,
	BashArithmeticForStatement,
	BashArithmeticStatement,
	BashArithmeticSubstitution,
	Bash53NativeSupershell,
	BashCaseInput,
	BashCasePattern,
	BashCasePatternHeader,
	BashCaseStatement,
	BashCommand,
	BashCommandSequence,
	BashForStatement,
	BashIfCondition,
	BashIfBranch,
	BashIfStatement,
	BashInCondition,
	BashPipeline,
	BashRedirection,
	BashSelectStatement,
	BashUntilStatement,
	BashVariable,
	BashWhileOrUntilCondition,
	BashWhileStatement,
	BashBreakOrContinueCommand,
	BashFunction,
	Block,
	ClassDefinition,
	ConstructorDefinition,
	DatamemberDeclaration,
	DeleteStatement,
	DestructorDefinition,
	DoublequotedString,
	DynamicCast,
	DynamicCastTarget,
	HeredocBody,
	HereString,
	IncludeStatement,
	MethodDefinition,
	NewStatement,
	ObjectAssignment,
	ObjectInstantiation,
	ObjectReference,
	ParameterExpansion,
	PrimitiveAssignment,
	ProcessSubstitution,
	RawSubshell,
	RawText,
	Rvalue,
	SubshellSubstitution,
	Supershell,
	TypeofExpression,
	ValueAssignment,
	ERROR_TYPE,
};
} // namespace bpp::AST
