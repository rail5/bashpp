/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_BASHPPLISTENER_H_
#define SRC_LISTENER_BASHPPLISTENER_H_

#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <memory>
#include <stack>
#include <optional>
#include <concepts>
#include <fstream>
#include <unordered_map>

#include "../AST/Listener/BaseListener.h"

class BashppListener;

#include "../bpp_include/bpp_codegen.h"
using code_segment = bpp::code_segment;
using bpp::generate_supershell_code;
using bpp::generate_delete_code;
using bpp::generate_method_call_code;
using bpp::generate_dynamic_cast_code;

#include "../bpp_include/bpp.h"
#include "../include/BashVersion.h"
#include "ContextExpectations.h"

#include "../syntax_error.h"
#include "../internal_error.h"

#define skip_syntax_errors if (error_thrown) { \
		if (error_node == node) { \
			error_thrown = false; \
			error_node = nullptr; \
		} \
		return; \
		}

template <typename T>
concept ASTNodePtrType = std::is_same_v<std::shared_ptr<AST::ASTNode>, T> ||
	std::is_base_of_v<AST::ASTNode, typename T::element_type>;

template <typename T>
concept ASTStringToken = std::is_same_v<AST::Token<std::string>, T>;

template <typename T>
concept ASTParameterToken = std::is_same_v<AST::Token<AST::MethodDefinition::Parameter>, T>;

template <typename T>
concept ASTNodePtrORToken = ASTNodePtrType<T> || ASTStringToken<T> || ASTParameterToken<T>;

/**
 * @class BashppListener
 * @brief The main listener class for the Bash++ compiler
 * 
 * This class is the main listener for the Bash++ compiler.
 * This is where the main logic for the compiler is implemented by walking the parse tree.
 * 
 * The listener is responsible for generating the compiled Bash code from the parsed Bash++ code.
 * The listener is also responsible for handling errors and warnings.
 * 
 * A brief run-down of how the parse tree works:
 * - The parse tree is a tree representation of the parsed code.
 * - When the parser encounters a new statement, it creates a new node in the parse tree.
 * - If Statement A is inside Statement B, then Statement A is a child of Statement B in the parse tree.
 * - The listener walks the parse tree, visiting each node in the tree in a depth-first manner.
 * - When the listener visits a node, it executes the corresponding function in the listener class.
 *
 * When we enter a node in the parse tree, we execute the enter* function for that node.
 * When we exit a node in the parse tree, we execute the exit* function for that node.
 */
class BashppListener : public AST::BaseListener<BashppListener>, std::enable_shared_from_this<BashppListener> {
	private:
		/**
		 * @var source_file
		 * @brief Path to the source file being compiled (used for error reporting)
		 */
		std::string source_file;

		bool included = false;

		/**
		 * @var include_paths
		 * @brief A list of paths to search for included files
		 */
		std::shared_ptr<std::vector<std::string>> include_paths = nullptr;

		bool suppress_warnings = false;

		/**
		 * @var included_files
		 * @brief A set of (unique) included files (used for '@include_once' directives)
		 */
		std::shared_ptr<std::set<std::string>> included_files = std::make_shared<std::set<std::string>>();
		BashppListener* included_from = nullptr;

		/**
		 * @var include_stack
		 * @brief A chain of included files, from the main file to the current file (used for error reporting)
		 */
		std::stack<std::string> include_stack;

		/**
		 * @var output_stream
		 * @brief Pointer to the output stream to write the compiled code to
		 */
		std::shared_ptr<std::ostream> code_buffer;
		std::shared_ptr<std::ostream> output_stream;
		std::string output_file;
		bool run_on_exit = false;

		/**
		* @var target_bash_version
		* @brief The target Bash version to compile for (default is 5.2)
		*/
		BashVersion target_bash_version = {5, 2};

		/**
		 * @var arguments
		 * @brief Command-line arguments to pass to the compiled program if run_on_exit is true
		 */
		std::vector<char*> arguments = {};

		std::shared_ptr<bpp::bpp_program> program = std::make_shared<bpp::bpp_program>();

		bool in_method = false;

		bool in_class = false;
		bool in_supershell = false;
		std::stack<std::monostate> bash_function_stack;
		bool should_declare_local() const;

		// Diagnostic information (not used for compilation):
		std::stack<std::monostate> dynamic_cast_stack; // Used to track whether we're inside a dynamic_cast statement
		std::stack<std::monostate> typeof_stack; // Used to track whether we're inside a typeof expression

		ExpectationsStack context_expectations_stack;

		/**
		 * @var entity_stack
		 * @brief A stack to keep track of the current entity being processed
		 * 
		 * For example, when we encounter a class definition, we push the class onto the entity_stack.
		 * Then, inside that class, when we encounter a method definition, we push the method onto the entity_stack.
		 * Inside that method, when we encounter a value assignment, we push the value assignment onto the entity_stack.
		 * When we're done with the value assignment, we pop it off the entity_stack, so that the method is now at the top of the stack.
		 * When we're done with the method, we pop it off the entity_stack, so that the class is now at the top of the stack.
		 * When we're done with the class, we pop it off the entity_stack, so that the program is now at the top of the stack.
		 */
		std::stack<std::shared_ptr<bpp::bpp_entity>> entity_stack;

		/**
		 * @var replacement_file_contents
		 * @brief A map of file paths to replacement contents for those files
		 * This is used by the language server to provide unsaved changes to the listener
		 * so that we can report diagnostics/completions/etc based on the unsaved changes
		 * 
		 */
		std::unordered_map<std::string, std::string> replacement_file_contents;

		std::shared_ptr<bpp::bpp_class> primitive;

		bool lsp_mode = false; // Whether this listener is just running as part of the language server (i.e., not really compiling anything)

		bool error_thrown = false;
		std::shared_ptr<AST::ASTNode> error_node = nullptr;

		bool program_has_errors = false;

		template <ASTNodePtrORToken T>
		void output_syntax_error_or_warning(const T& error_ctx, const std::string& msg, bool is_warning = false) {
			uint32_t line;
			uint32_t column;
			uint32_t text_length = 1;

			if constexpr (ASTNodePtrType<T>) {
				line = error_ctx->getPosition().line;
				column = error_ctx->getPosition().column;
				if (error_ctx->getEndPosition().line == error_ctx->getPosition().line) {
					text_length = error_ctx->getEndPosition().column - error_ctx->getPosition().column;
				} else {
					// Multi-line node: Just highlight 1 character
					text_length = 1;
				}
			} else if constexpr (ASTStringToken<T>) {
				line = error_ctx.getLine();
				column = error_ctx.getCharPositionInLine();
				text_length = static_cast<uint32_t>(error_ctx.getValue().length());
			} else if constexpr (ASTParameterToken<T>) {
				// Special case: Error reporting on a declared method parameter
				// FIXME(@rail5): Kind of hacky to handle special cases. Would prefer a general solution.
				line = error_ctx.getLine();
				column = error_ctx.getCharPositionInLine();
				auto param = error_ctx.getValue();
				text_length = 0;
				if (param.type.has_value()) {
					text_length += 1 + static_cast<uint32_t>(param.type.value().getValue().length()); // '@Type'
					if (param.pointer) text_length += 1; // '*'
					text_length += 1 + static_cast<uint32_t>(param.name.getValue().length()); // ' Name'
				} else {
					text_length = static_cast<uint32_t>(param.name.getValue().length()); // 'Name'
				}
			}
			print_syntax_error_or_warning(source_file, line, column, text_length, msg, get_include_stack(), program, lsp_mode, is_warning);
			program_has_errors = !is_warning;
		}

		#define throw_syntax_error(token, msg) \
			output_syntax_error_or_warning(token, msg); \
			error_thrown = true; \
			error_node = node; \
			node->clearChildren(); /* Skip traversing children */ \
			return;

		#define throw_syntax_error_from_exitRule(token, msg) \
			output_syntax_error_or_warning(token, msg); \
			return;
		
		#define show_warning(token, msg) \
			if (!suppress_warnings) { \
				output_syntax_error_or_warning(token, msg, true); \
			}

	public:

	void set_source_file(std::string source_file);
	void set_include_paths(std::shared_ptr<std::vector<std::string>> include_paths);
	void set_included(bool included);
	void set_included_from(BashppListener* included_from);
	void set_included_files(std::shared_ptr<std::set<std::string>> included_files);
	void set_errors();
	void set_code_buffer(std::shared_ptr<std::ostream> code_buffer);
	void set_output_stream(std::shared_ptr<std::ostream> output_stream);
	void set_output_file(std::string output_file);
	void set_run_on_exit(bool run_on_exit);
	void set_suppress_warnings(bool suppress_warnings);
	void set_target_bash_version(BashVersion target_bash_version);
	void set_arguments(std::vector<char*> arguments);
	void set_lsp_mode(bool lsp_mode);

	void set_replacement_file_contents(const std::string& file_path, const std::string& contents);

	std::shared_ptr<bpp::bpp_program> get_program();
	std::shared_ptr<std::set<std::string>> get_included_files();
	std::stack<std::string> get_include_stack();

	std::shared_ptr<bpp::bpp_code_entity> latest_code_entity();

	void enterProgram(std::shared_ptr<AST::Program> node);
	void exitProgram(std::shared_ptr<AST::Program> node);
	void enterArrayAssignment(std::shared_ptr<AST::ArrayAssignment> node);
	void exitArrayAssignment(std::shared_ptr<AST::ArrayAssignment> node);
	void enterArrayIndex(std::shared_ptr<AST::ArrayIndex> node);
	void exitArrayIndex(std::shared_ptr<AST::ArrayIndex> node);
	void enterBash53NativeSupershell(std::shared_ptr<AST::Bash53NativeSupershell> node);
	void exitBash53NativeSupershell(std::shared_ptr<AST::Bash53NativeSupershell> node);
	void enterBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node);
	void exitBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node);
	void enterBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node);
	void exitBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node);
	void enterBashArithmeticSubstitution(std::shared_ptr<AST::BashArithmeticSubstitution> node);
	void exitBashArithmeticSubstitution(std::shared_ptr<AST::BashArithmeticSubstitution> node);
	void enterBashCasePattern(std::shared_ptr<AST::BashCasePattern> node);
	void exitBashCasePattern(std::shared_ptr<AST::BashCasePattern> node);
	void enterBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node);
	void exitBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node);
	void enterBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node);
	void exitBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node);
	//void enterBashCommand(std::shared_ptr<AST::BashCommand> node);
	//void exitBashCommand(std::shared_ptr<AST::BashCommand> node);
	void enterBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node);
	void exitBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node);
	void enterBashForStatement(std::shared_ptr<AST::BashForStatement> node);
	void exitBashForStatement(std::shared_ptr<AST::BashForStatement> node);
	void enterBashIfCondition(std::shared_ptr<AST::BashIfCondition> node);
	void exitBashIfCondition(std::shared_ptr<AST::BashIfCondition> node);
	void enterBashIfElseBranch(std::shared_ptr<AST::BashIfElseBranch> node);
	void exitBashIfElseBranch(std::shared_ptr<AST::BashIfElseBranch> node);
	void enterBashIfRootBranch(std::shared_ptr<AST::BashIfRootBranch> node);
	void exitBashIfRootBranch(std::shared_ptr<AST::BashIfRootBranch> node);
	void enterBashIfStatement(std::shared_ptr<AST::BashIfStatement> node);
	void exitBashIfStatement(std::shared_ptr<AST::BashIfStatement> node);
	void enterBashInCondition(std::shared_ptr<AST::BashInCondition> node);
	void exitBashInCondition(std::shared_ptr<AST::BashInCondition> node);
	void enterBashPipeline(std::shared_ptr<AST::BashPipeline> node);
	void exitBashPipeline(std::shared_ptr<AST::BashPipeline> node);
	void enterBashRedirection(std::shared_ptr<AST::BashRedirection> node);
	void exitBashRedirection(std::shared_ptr<AST::BashRedirection> node);
	void enterBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node);
	void exitBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node);
	void enterBashTestConditionCommand(std::shared_ptr<AST::BashTestConditionCommand> node);
	void exitBashTestConditionCommand(std::shared_ptr<AST::BashTestConditionCommand> node);
	void enterBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node);
	void exitBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node);
	void enterBashVariable(std::shared_ptr<AST::BashVariable> node);
	void exitBashVariable(std::shared_ptr<AST::BashVariable> node);
	void enterBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node);
	void exitBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node);
	void enterBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node);
	void exitBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node);
	void enterBashFunction(std::shared_ptr<AST::BashFunction> node);
	void exitBashFunction(std::shared_ptr<AST::BashFunction> node);
	void enterBlock(std::shared_ptr<AST::Block> node);
	void exitBlock(std::shared_ptr<AST::Block> node);
	void enterClassDefinition(std::shared_ptr<AST::ClassDefinition> node);
	void exitClassDefinition(std::shared_ptr<AST::ClassDefinition> node);
	void enterConnective(std::shared_ptr<AST::Connective> node);
	void exitConnective(std::shared_ptr<AST::Connective> node);
	void enterConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node);
	void exitConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node);
	void enterDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node);
	void exitDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node);
	void enterDeleteStatement(std::shared_ptr<AST::DeleteStatement> node);
	void exitDeleteStatement(std::shared_ptr<AST::DeleteStatement> node);
	void enterDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node);
	void exitDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node);
	void enterDoublequotedString(std::shared_ptr<AST::DoublequotedString> node);
	void exitDoublequotedString(std::shared_ptr<AST::DoublequotedString> node);
	void enterDynamicCast(std::shared_ptr<AST::DynamicCast> node);
	void exitDynamicCast(std::shared_ptr<AST::DynamicCast> node);
	void enterDynamicCastTarget(std::shared_ptr<AST::DynamicCastTarget> node);
	void exitDynamicCastTarget(std::shared_ptr<AST::DynamicCastTarget> node);
	void enterHeredocBody(std::shared_ptr<AST::HeredocBody> node);
	void exitHeredocBody(std::shared_ptr<AST::HeredocBody> node);
	void enterHereString(std::shared_ptr<AST::HereString> node);
	void exitHereString(std::shared_ptr<AST::HereString> node);
	void enterIncludeStatement(std::shared_ptr<AST::IncludeStatement> node);
	void exitIncludeStatement(std::shared_ptr<AST::IncludeStatement> node);
	void enterMethodDefinition(std::shared_ptr<AST::MethodDefinition> node);
	void exitMethodDefinition(std::shared_ptr<AST::MethodDefinition> node);
	void enterNewStatement(std::shared_ptr<AST::NewStatement> node);
	void exitNewStatement(std::shared_ptr<AST::NewStatement> node);
	void enterObjectAssignment(std::shared_ptr<AST::ObjectAssignment> node);
	void exitObjectAssignment(std::shared_ptr<AST::ObjectAssignment> node);
	void enterObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node);
	void exitObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node);
	void enterObjectReference(std::shared_ptr<AST::ObjectReference> node);
	void exitObjectReference(std::shared_ptr<AST::ObjectReference> node);
	void enterParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node);
	void exitParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node);
	void enterPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node);
	void exitPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node);
	void enterPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node);
	void exitPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node);
	void enterProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node);
	void exitProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node);
	void enterRawSubshell(std::shared_ptr<AST::RawSubshell> node);
	void exitRawSubshell(std::shared_ptr<AST::RawSubshell> node);
	void enterRawText(std::shared_ptr<AST::RawText> node);
	void exitRawText(std::shared_ptr<AST::RawText> node);
	//void enterRvalue(std::shared_ptr<AST::Rvalue> node);
	//void exitRvalue(std::shared_ptr<AST::Rvalue> node);
	void enterSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node);
	void exitSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node);
	void enterSupershell(std::shared_ptr<AST::Supershell> node);
	void exitSupershell(std::shared_ptr<AST::Supershell> node);
	void enterTypeofExpression(std::shared_ptr<AST::TypeofExpression> node);
	void exitTypeofExpression(std::shared_ptr<AST::TypeofExpression> node);
	void enterValueAssignment(std::shared_ptr<AST::ValueAssignment> node);
	void exitValueAssignment(std::shared_ptr<AST::ValueAssignment> node);

};

#endif // SRC_LISTENER_BASHPPLISTENER_H_
