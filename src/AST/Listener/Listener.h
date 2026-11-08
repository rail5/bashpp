/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <stack>
#include <set>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include <AST/ASTNode.h>
#include <AST/NodeTypes.h>
#include <AST/Nodes/Nodes.h>
#include <error/WarningOptions.h>
#include <error/detail.h>
#include <IR/SourceIndex.h>

#include "ContextExpectations.h"
#include "EntityStack.h"

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
	X(BashIfBranch) \
	X(BashIfStatement) \
	X(BashInCondition) \
	X(BashPipeline) \
	X(BashRedirection) \
	X(BashSelectStatement) \
	X(BashUntilStatement) \
	X(BashVariable) \
	X(BashWhileOrUntilCondition) \
	X(BashWhileStatement) \
	X(BashBreakOrContinueCommand) \
	X(Block) \
	X(ClassDefinition) \
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
	public:
		void walk(bpp::AST::ASTNode* node);

		bpp::IR::Program* view_program() const { return program.get(); }
		std::unique_ptr<bpp::IR::Program> release_program();

		// Default (empty) implementations of enter/exit for each node type.
		// Specializations are provided for node types that need to be handled.
		template <typename NodeType>
		void enter(NodeType* node) {}

		template <typename NodeType>
		void exit(NodeType* node) {}

		void setHasErrors(bool has_errors) { this->program_has_errors = has_errors; }
		bool hasErrors() const { return program_has_errors; }
		void setUTF16Mode(bool is_utf16) { this->utf16_mode = is_utf16; }
		bool isUTF16Mode() const { return this->utf16_mode; }
		void setReplacementFileContents(const std::filesystem::path& file_path, const std::string& contents) {
			replacement_file_contents[file_path] = contents;
		}
		void setReplacementFileContents(const std::unordered_map<std::filesystem::path, std::string>& replacements) {
			replacement_file_contents = replacements;
		}

		/// @brief Request that this listener prepare a SourceIndex for use in diagnostics and language server support.
		/// @note This should be called BEFORE @c walk()
		void prepareSourceIndex() {
			if (!source_index) source_index = std::make_unique<bpp::IR::SourceIndex>();
		}
		std::unique_ptr<bpp::IR::SourceIndex> releaseSourceIndex() {
			return std::move(source_index);
		}

		void setSourceFile(const std::filesystem::path& source_file) {
			if (!include_chain.empty()) throw std::runtime_error("Source file already set; cannot set source file when include chain is not empty");
			include_chain.emplace_back(source_file);
		}
		const std::filesystem::path& getMainSourceFile() const {
			return include_chain.front();
		}
		const std::filesystem::path& getCurrentSourceFile() const {
			return include_chain.back();
		}
		const std::vector<std::filesystem::path>& getIncludeChain() const {
			return include_chain;
		}

		void setWarningOptions(const bpp::ErrorHandling::WarningOptions& options) {
			this->warning_options = options;
		}
		const bpp::ErrorHandling::WarningOptions& getWarningOptions() const {
			return warning_options;
		}
		void setIncludePaths(const std::vector<std::filesystem::path>& paths) {
			this->include_paths = paths;
			this->include_paths.emplace_back(bpp::get_standard_library_path());
		}
	private:
		/// The program (root node of the entity tree) being constructed by this listener
		std::unique_ptr<bpp::IR::Program> program;
		bpp::IR::Program* root_program = nullptr; // Non-owning pointer to the root program

		bool program_has_errors = false;

		/**
		 * @brief Whether character counting is done in UTF-16 mode (for LSP support) or UTF-8 mode (for normal operation).
		 * All input files are expected to be UTF-8 encoded, with no exceptions.
		 * Strangely, the LSP spec mandates that UTF-16 code units be supported by all language servers,
		 * with UTF-8 being an "optional extension." It doesn't really make any sense,
		 * but we have to support it for LSP clients to work properly.
		 *
		 * This flag is stored in the Listener so that it can propagate to new lexers/parsers spawned in IncludeStatements.
		 */
		bool utf16_mode = false;

		/**
		 * @brief A map of file paths to replacement file contents:
		 * When a new parser is spawned, if the file path is in this map, the parser will use the replacement contents instead of reading from the file system.
		 * This is only used by the language server. It allows the server to provide diagnostics based on unsaved changes in the editor.
		 */
		std::unordered_map<std::filesystem::path, std::string> replacement_file_contents;

		/**
		 * @brief A searchable index of entity information across source files, used for diagnostics and language server support.
		 * This structure provides:
		 *  - Definition positions for entities
		 *  - Reference positions for entities
		 *  - Maps of source files by "context" entities (code containers such as classes, methods, and blocks)
		 *  - Diagnostics (warnings, errors) for source files
		 *  - A list of all source files that have been processed (including included files)
		 *
		 * This index is only built if explicitly requested, as it is not needed for normal operation of the compiler.
		 */
		std::unique_ptr<bpp::IR::SourceIndex> source_index;

		/// The set of enabled/disabled warnings
		bpp::ErrorHandling::WarningOptions warning_options;

		/**
		 * @brief Show a warning message to the user, if the warning type is enabled.
		 * If the warning type is disabled, this does nothing.
		 * If -Werror is enabled, this will throw the warning as an error instead of simply printing it.
		 * If the Listener is building a @c SourceIndex, the warning will also be added to the index.
		 *
		 * @tparam T The type of the AST node or token that produced the warning
		 * @param node The AST node or token that produced the warning
		 * @param warning_type The type of warning to show
		 * @param msg The warning message to display
		 * @throws bpp::ErrorHandling::Diagnostic if the warning is treated as an error due to -Werror
		 * @note This should only be called from within the Listener's enter/exit methods,
		 *       as it relies on the Listener's include chain to determine the source file of the warning.
		 */
		template <bpp::detail::ASTNodePtrORToken T>
		void warn(const T& node, bpp::ErrorHandling::WarningType warning_type, const std::string& msg) {
			if (warning_options.is_enabled(warning_type)) {
				bpp::ErrorHandling::Diagnostic warning(msg, include_chain, node, bpp::ErrorHandling::DiagnosticType::DIAGNOSTIC_WARNING, warning_options.get_cli_string_by_option(warning_type));
				if (source_index) source_index->addDiagnostic(include_chain.back(), warning);

				if (warning_options.is_enabled(bpp::ErrorHandling::WarningType::WarningsAsErrors)) {
					warning.setType(bpp::ErrorHandling::DiagnosticType::DIAGNOSTIC_ERROR);
					throw std::move(warning);
				} else {
					warning.print_to_stderr();
				}
			}
		}

		/**
		 * @brief Construct and throw a Diagnostic error based on the given AST node or token.
		 * If the Listener is building a @c SourceIndex, the error will also be added to the index.
		 *
		 * @tparam T The type of the AST node or token that produced the error
		 * @param node The AST node or token that produced the error
		 * @param msg The error message to display
		 * @throws bpp::ErrorHandling::Diagnostic always
		 * @note This should only be called from within the Listener's enter/exit methods,
		 *       as it relies on the Listener's include chain to determine the source file of the error.
		 */
		template <bpp::detail::ASTNodePtrORToken T>
		void error(const T& node, const std::string& msg) {
			bpp::ErrorHandling::Diagnostic error(msg, include_chain, node, bpp::ErrorHandling::DiagnosticType::DIAGNOSTIC_ERROR);
			if (source_index) source_index->addDiagnostic(include_chain.back(), error);
			throw std::move(error);
		}

		enum class IncludedType : std::uint8_t {
			NOT_INCLUDED, // The file that generated this AST is the original (main) source file of the program
			DYNAMICALLY_INCLUDED, // This file was reached via `@include dynamic <file>`
			STATICALLY_INCLUDED, // This file was reached via `@include [static] <file>`
		};

		std::stack<IncludedType> included_type_stack = std::stack<IncludedType>({IncludedType::NOT_INCLUDED});

		/// A set of (unique) included files (used to avoid including the same file multiple times, unless `@include_always` is given)
		std::set<std::filesystem::path> included_files;

		/// A list of paths to search for angle-bracket included files (e.g., `@include <file>`). The last path is always the standard library path.
		std::vector<std::filesystem::path> include_paths;

		/// A chain of included files, from the original main file to the current file being processed
		std::vector<std::filesystem::path> include_chain;

		bpp::IR::EntityStack entity_stack;

		/**
		 * @brief Helper function to check the type of the top of the entity stack
		 *
		 * @tparam T The type to check against
		 * @return true if the top of the entity stack is of type T (or derived from T)
		 * @return false if the top of the entity stack is not of type T and not derived from T
		 */
		template <class T>
		bool topmost_entity_is() const { return entity_stack.top_is<T>(); }

		/**
		 * @brief Get the latest code entity on the entity stack, or nullptr if there is none
		 *
		 * This traverses the stack from top to bottom, returning the first entity that is a CodeEntity (or derived from CodeEntity).
		 * The entity returned may not be the topmost entity on the stack.
		 * 
		 * @return bpp::IR::CodeEntity* The latest code entity on the stack, or nullptr if there is none
		 */
		bpp::IR::CodeEntity* latest_code_entity() const;

		ExpectationsStack context_expectations_stack;
};

// Enter/exit handler specializations:
template<> void Listener::enter  (Program*                    node);
template<> void Listener::exit   (Program*                    node);
template<> void Listener::enter  (IncludeStatement*           node);
template<> void Listener::exit   (IncludeStatement*           node);
template<> void Listener::enter  (ClassDefinition*            node);
template<> void Listener::exit   (ClassDefinition*            node);
template<> void Listener::enter  (DatamemberDeclaration*      node);
template<> void Listener::exit   (DatamemberDeclaration*      node);
template<> void Listener::enter  (ValueAssignment*            node);
template<> void Listener::exit   (ValueAssignment*            node);
template<> void Listener::enter  (MethodDefinition*           node);
template<> void Listener::exit   (MethodDefinition*           node);
template<> void Listener::enter  (ConstructorDefinition*      node);
template<> void Listener::exit   (ConstructorDefinition*      node);
template<> void Listener::enter  (DestructorDefinition*       node);
template<> void Listener::exit   (DestructorDefinition*       node);
template<> void Listener::enter  (ObjectInstantiation*        node);
template<> void Listener::exit   (ObjectInstantiation*        node);
template<> void Listener::enter  (ObjectAssignment*           node);
template<> void Listener::exit   (ObjectAssignment*           node);
template<> void Listener::enter  (NewStatement*               node);
template<> void Listener::exit   (NewStatement*               node);
template<> void Listener::enter  (DeleteStatement*            node);
template<> void Listener::exit   (DeleteStatement*            node);
template<> void Listener::enter  (ObjectReference*            node);
template<> void Listener::exit   (ObjectReference*            node);
template<> void Listener::enter  (DynamicCast*                node);
template<> void Listener::exit   (DynamicCast*                node);
template<> void Listener::enter  (DynamicCastTarget*          node);
template<> void Listener::exit   (DynamicCastTarget*          node);
template<> void Listener::enter  (TypeofExpression*           node);
template<> void Listener::exit   (TypeofExpression*           node);
template<> void Listener::enter  (BashCommand*                node);
template<> void Listener::exit   (BashCommand*                node);
template<> void Listener::enter  (BashPipeline*               node);
template<> void Listener::exit   (BashPipeline*               node);
template<> void Listener::enter  (BashCommandSequence*        node);
template<> void Listener::exit   (BashCommandSequence*        node);
template<> void Listener::enter  (BashBreakOrContinueCommand* node);
template<> void Listener::exit   (BashBreakOrContinueCommand* node);
template<> void Listener::enter  (BashIfStatement*            node);
template<> void Listener::exit   (BashIfStatement*            node);
template<> void Listener::enter  (BashIfCondition*            node);
template<> void Listener::exit   (BashIfCondition*            node);
template<> void Listener::enter  (BashIfBranch*               node);
template<> void Listener::exit   (BashIfBranch*               node);
template<> void Listener::enter  (BashWhileStatement*         node);
template<> void Listener::exit   (BashWhileStatement*         node);
template<> void Listener::enter  (BashUntilStatement*         node);
template<> void Listener::exit   (BashUntilStatement*         node);
template<> void Listener::enter  (BashWhileOrUntilCondition*  node);
template<> void Listener::exit   (BashWhileOrUntilCondition*  node);
template<> void Listener::enter  (BashCaseStatement*          node);
template<> void Listener::exit   (BashCaseStatement*          node);
template<> void Listener::enter  (BashCaseInput*              node);
template<> void Listener::exit   (BashCaseInput*              node);
template<> void Listener::enter  (BashCasePattern*            node);
template<> void Listener::exit   (BashCasePattern*            node);
template<> void Listener::enter  (BashCasePatternHeader*      node);
template<> void Listener::exit   (BashCasePatternHeader*      node);
template<> void Listener::enter  (BashForStatement*           node);
template<> void Listener::exit   (BashForStatement*           node);
template<> void Listener::enter  (BashSelectStatement*        node);
template<> void Listener::exit   (BashSelectStatement*        node);
template<> void Listener::enter  (BashInCondition*            node);
template<> void Listener::exit   (BashInCondition*            node);
template<> void Listener::enter  (BashArithmeticForStatement* node);
template<> void Listener::exit   (BashArithmeticForStatement* node);
template<> void Listener::enter  (BashArithmeticForCondition* node);
template<> void Listener::exit   (BashArithmeticForCondition* node);
template<> void Listener::enter  (BashFunction*               node);
template<> void Listener::exit   (BashFunction*               node);
template<> void Listener::enter  (RawText*                    node);
template<> void Listener::exit   (RawText*                    node);
template<> void Listener::enter  (DoublequotedString*         node);
template<> void Listener::exit   (DoublequotedString*         node);
template<> void Listener::enter  (Supershell*                 node);
template<> void Listener::exit   (Supershell*                 node);
template<> void Listener::enter  (SubshellSubstitution*       node);
template<> void Listener::exit   (SubshellSubstitution*       node);
template<> void Listener::enter  (RawSubshell*                node);
template<> void Listener::exit   (RawSubshell*                node);
template<> void Listener::enter  (Bash53NativeSupershell*     node);
template<> void Listener::exit   (Bash53NativeSupershell*     node);
template<> void Listener::enter  (BashRedirection*            node);
template<> void Listener::exit   (BashRedirection*            node);
template<> void Listener::enter  (ProcessSubstitution*        node);
template<> void Listener::exit   (ProcessSubstitution*        node);
template<> void Listener::enter  (HeredocBody*                node);
template<> void Listener::exit   (HeredocBody*                node);
template<> void Listener::enter  (HereString*                 node);
template<> void Listener::exit   (HereString*                 node);
template<> void Listener::enter  (BashVariable*               node);
template<> void Listener::exit   (BashVariable*               node);
template<> void Listener::enter  (PrimitiveAssignment*        node);
template<> void Listener::exit   (PrimitiveAssignment*        node);
template<> void Listener::enter  (BashArithmeticSubstitution* node);
template<> void Listener::exit   (BashArithmeticSubstitution* node);
template<> void Listener::enter  (ArrayIndex*                 node);
template<> void Listener::exit   (ArrayIndex*                 node);
template<> void Listener::enter  (ArrayAssignment*            node);
template<> void Listener::exit   (ArrayAssignment*            node);
template<> void Listener::enter  (ParameterExpansion*         node);
template<> void Listener::exit   (ParameterExpansion*         node);
template<> void Listener::enter  (Block*                      node);
template<> void Listener::exit   (Block*                      node);

} // namespace bpp::AST
