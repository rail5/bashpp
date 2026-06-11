/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>

#include <AST/ASTNode.h>
#include <AST/NodeTypes.h>
#include <AST/Nodes/Nodes.h>
#include <error/InternalError.h>
#include <error/SyntaxError.h>
#include <error/ParserError.h>

#include <IR/bpp.h>

namespace bpp::AST {

#define AST_LISTENER_NODE_LIST(X) \
	X(ArrayAssignment) \
	X(ArrayIndex) \
	X(Bash53NativeSupershell) \
	X(BashArithmeticForCondition) \
	X(BashArithmeticForStatement) \
	X(BashArithmeticStatement) \
	X(BashArithmeticSubstitution) \
	X(BashCaseInput) \
	X(BashCasePattern) \
	X(BashCasePatternHeader) \
	X(BashCaseStatement) \
	X(BashCommand) \
	X(BashCommandSequence) \
	X(BashForStatement) \
	X(BashFunction) \
	X(BashIfCondition) \
	X(BashIfElseBranch) \
	X(BashIfRootBranch) \
	X(BashIfStatement) \
	X(BashInCondition) \
	X(BashPipeline) \
	X(BashRedirection) \
	X(BashSelectStatement) \
	X(BashTestConditionCommand) \
	X(BashUntilStatement) \
	X(BashVariable) \
	X(BashWhileOrUntilCondition) \
	X(BashWhileStatement) \
	X(Block) \
	X(ClassDefinition) \
	X(Connective) \
	X(ConstructorDefinition) \
	X(DatamemberDeclaration) \
	X(DeleteStatement) \
	X(DestructorDefinition) \
	X(DoublequotedString) \
	X(DynamicCast) \
	X(DynamicCastTarget) \
	X(HeredocBody) \
	X(HereString) \
	X(IncludeStatement) \
	X(MethodDefinition) \
	X(NewStatement) \
	X(ObjectAssignment) \
	X(ObjectInstantiation) \
	X(ObjectReference) \
	X(ParameterExpansion) \
	X(PointerDeclaration) \
	X(PrimitiveAssignment) \
	X(ProcessSubstitution) \
	X(Program) \
	X(RawSubshell) \
	X(RawText) \
	X(Rvalue) \
	X(SubshellSubstitution) \
	X(Supershell) \
	X(TypeofExpression) \
	X(ValueAssignment) \

class Listener {
	private:
		bool program_has_errors = false;
		std::vector<bpp::AST::ParserError> parser_errors;

	public:
		void walk(std::shared_ptr<bpp::AST::ASTNode> node);

		// Default (empty) implementations of enter/exit for each node type.
		// Specializations are provided for node types that need to be handled.
		template <typename NodeType>
		void enter(std::shared_ptr<NodeType> node) {}

		template <typename NodeType>
		void exit(std::shared_ptr<NodeType> node) {}

		void set_has_errors(bool has_errors) {
			this->program_has_errors = has_errors;
		}

		void set_parser_errors(const std::vector<bpp::AST::ParserError>& errors) {
			this->parser_errors = errors;
			if (!errors.empty()) this->program_has_errors = true;
		}
};

// Enter/exit handler specializations:
template <> void Listener::enter(std::shared_ptr<Program> node);
template <> void Listener::exit(std::shared_ptr<Program> node);

} // namespace bpp::AST
