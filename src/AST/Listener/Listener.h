/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <stack>
#include <set>
#include <filesystem>

#include <AST/ASTNode.h>
#include <AST/NodeTypes.h>
#include <AST/Nodes/Nodes.h>
#include <error/ParserError.h>
#include <error/WarningOptions.h>
#include <error/detail.h>

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
		/// The program (root node of the entity tree) being constructed by this listener
		std::shared_ptr<bpp::IR::Program> program;

		bool program_has_errors = false;
		std::vector<bpp::AST::ParserError> parser_errors;

		/// The set of enabled/disabled warnings
		bpp::ErrorHandling::WarningOptions warning_options;
		#define show_warning(node, warning_type, msg) \
			if (warning_options.is_enabled(warning_type)) { \
				bpp::ErrorHandling::Warning warning(this, node, msg, warning_type); \
				warning.print(); \
			}

		enum class IncludedType : std::uint8_t {
			NOT_INCLUDED, // The file that generated this AST is the original (main) source file of the program
			DYNAMICALLY_INCLUDED, // This file was reached via `@include dynamic <file>`
			STATICALLY_INCLUDED // This file was reached via `@include [static] <file>`
		};

		std::stack<IncludedType> included_type_stack = std::stack<IncludedType>({IncludedType::NOT_INCLUDED});

		/// A set of (unique) included files (used for `@include_once` to avoid including the same file multiple times)
		std::set<std::filesystem::path> included_files;

		/// A list of paths to search for angle-bracket included files (e.g., `@include <file>`). The last path is always the standard library path.
		std::vector<std::filesystem::path> include_paths;

		/// A chain of included files, from the original main file to the current file being processed
		std::vector<std::filesystem::path> include_chain;

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

		std::stack<std::monostate> dynamic_cast_stack;
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
		bool has_errors() const {
			return program_has_errors;
		}

		void set_parser_errors(const std::vector<bpp::AST::ParserError>& errors) {
			this->parser_errors = errors;
			if (!errors.empty()) this->program_has_errors = true;
		}

		void set_source_file(const std::filesystem::path& source_file) {
			if (!include_chain.empty()) throw std::runtime_error("Source file already set; cannot set source file when include chain is not empty");
			include_chain.emplace_back(source_file);
		}
		const std::filesystem::path& get_main_source_file() const {
			return include_chain.front();
		}
		const std::filesystem::path& get_current_source_file() const {
			return include_chain.back();
		}
		const std::vector<std::filesystem::path>& get_include_chain() const {
			return include_chain;
		}

		void set_warning_options(const bpp::ErrorHandling::WarningOptions& options) {
			this->warning_options = options;
		}
		const bpp::ErrorHandling::WarningOptions& get_warning_options() const {
			return warning_options;
		}
		void set_include_paths(const std::vector<std::filesystem::path>& paths) {
			this->include_paths = paths;
		}
};

// Enter/exit handler specializations:
template<> void Listener::enter  (Program*                node);
template<> void Listener::exit   (Program*                node);
template<> void Listener::enter  (IncludeStatement*       node);
template<> void Listener::exit   (IncludeStatement*       node);
template<> void Listener::enter  (ClassDefinition*        node);
template<> void Listener::exit   (ClassDefinition*        node);
template<> void Listener::enter  (DatamemberDeclaration*  node);
template<> void Listener::exit   (DatamemberDeclaration*  node);
template<> void Listener::enter  (ValueAssignment*        node);
template<> void Listener::exit   (ValueAssignment*        node);
template<> void Listener::enter  (MethodDefinition*       node);
template<> void Listener::exit   (MethodDefinition*       node);
template<> void Listener::enter  (ObjectInstantiation*    node);
template<> void Listener::exit   (ObjectInstantiation*    node);
template<> void Listener::enter  (DynamicCast*            node);
template<> void Listener::exit   (DynamicCast*            node);
template<> void Listener::enter  (DynamicCastTarget*      node);
template<> void Listener::exit   (DynamicCastTarget*      node);
template<> void Listener::enter  (BashPipeline*           node);
template<> void Listener::exit   (BashPipeline*           node);
template<> void Listener::enter  (BashCommandSequence*    node);
template<> void Listener::exit   (BashCommandSequence*    node);
template<> void Listener::enter  (RawText*                node);
template<> void Listener::exit   (RawText*                node);
template<> void Listener::enter  (DoublequotedString*     node);
template<> void Listener::exit   (DoublequotedString*     node);
template<> void Listener::enter  (Supershell*             node);
template<> void Listener::exit   (Supershell*             node);
template<> void Listener::enter  (SubshellSubstitution*   node);
template<> void Listener::exit   (SubshellSubstitution*   node);
template<> void Listener::enter  (RawSubshell*            node);
template<> void Listener::exit   (RawSubshell*            node);
template<> void Listener::enter  (Bash53NativeSupershell* node);
template<> void Listener::exit   (Bash53NativeSupershell* node);
template<> void Listener::enter  (BashRedirection*        node);
template<> void Listener::exit   (BashRedirection*        node);
template<> void Listener::enter  (BashVariable*           node);
template<> void Listener::exit   (BashVariable*           node);

} // namespace bpp::AST
