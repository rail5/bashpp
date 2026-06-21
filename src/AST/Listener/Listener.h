/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <stack>

#include <AST/ASTNode.h>
#include <AST/NodeTypes.h>
#include <AST/Nodes/Nodes.h>
#include <error/InternalError.h>
#include <error/SyntaxError.h>
#include <error/ParserError.h>

#include "ContextExpectations.h"

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

class Listener final {
	private:
		/// Path to the source file that generated the AST being traversed by this listener
		std::string source_file;

		/// The program (root node of the entity tree) being constructed by this listener
		std::shared_ptr<bpp::IR::Program> program;

		bool program_has_errors = false;
		std::vector<bpp::AST::ParserError> parser_errors;

		enum class IncludedType : uint8_t {
			NOT_INCLUDED, // The file that generated this AST is the original (main) source file of the program
			DYNAMICALLY_INCLUDED, // This file was reached via `@include dynamic <file>`
			STATICALLY_INCLUDED // This file was reached via `@include [static] <file>`
		} included_type = IncludedType::NOT_INCLUDED;

		bool is_included() const { return included_type != IncludedType::NOT_INCLUDED; }

		std::stack<std::shared_ptr<bpp::IR::Entity>> entity_stack;

		/**
		 * @brief Helper function to check the type of the top of the entity stack
		 *
		 * @tparam T The type to check against
		 * @return true if the top of the entity stack is of type T (or derived from T)
		 * @return false if the top of the entity stack is not of type T and not derived from T
		 */
		template <class T>
		bool topmost_entity_is() const {
			if (entity_stack.empty()) return false;
			return std::dynamic_pointer_cast<T>(entity_stack.top()) != nullptr;
		}

		/**
		 * @brief Get the latest code entity on the entity stack, or nullptr if there is none
		 *
		 * This traverses the stack from top to bottom, returning the first entity that is a CodeEntity (or derived from CodeEntity).
		 * The entity returned may not be the topmost entity on the stack.
		 * 
		 * @return std::shared_ptr<bpp::IR::Entity> The latest code entity on the stack, or nullptr if there is none
		 */
		std::shared_ptr<bpp::IR::CodeEntity> latest_code_entity() const;

		bool in_class = false;
		bool in_method = false;

		ExpectationsStack context_expectations_stack;

	public:
		void walk(bpp::AST::ASTNode* node);

		std::shared_ptr<bpp::IR::Program> get_program() const { return program; }

		// Default (empty) implementations of enter/exit for each node type.
		// Specializations are provided for node types that need to be handled.
		template <typename NodeType>
		void enter(NodeType* node) {}

		template <typename NodeType>
		void exit(NodeType* node) {}

		void set_has_errors(bool has_errors) {
			this->program_has_errors = has_errors;
		}

		void set_parser_errors(const std::vector<bpp::AST::ParserError>& errors) {
			this->parser_errors = errors;
			if (!errors.empty()) this->program_has_errors = true;
		}

		void set_source_file(const std::string& source_file) {
			this->source_file = source_file;
		}
};

// Enter/exit handler specializations:
template <> void Listener::enter(Program* node);
template <> void Listener::exit(Program* node);

template <> void Listener::enter(ClassDefinition* node);
template <> void Listener::exit(ClassDefinition* node);

template <> void Listener::enter(DatamemberDeclaration* node);
template <> void Listener::exit(DatamemberDeclaration* node);

template <> void Listener::enter(ValueAssignment* node);
template <> void Listener::exit(ValueAssignment* node);

template <> void Listener::enter(MethodDefinition* node);
template <> void Listener::exit(MethodDefinition* node);

template <> void Listener::enter(ObjectInstantiation* node);
template <> void Listener::exit(ObjectInstantiation* node);

template <> void Listener::enter(BashCommand* node);
template <> void Listener::exit(BashCommand* node);

template <> void Listener::enter(BashPipeline* node);
template <> void Listener::exit(BashPipeline* node);

template <> void Listener::enter(BashCommandSequence* node);
template <> void Listener::exit(BashCommandSequence* node);

template <> void Listener::enter(RawText* node);
template <> void Listener::exit(RawText* node);

template <> void Listener::enter(DoublequotedString* node);
template <> void Listener::exit(DoublequotedString* node);

} // namespace bpp::AST
