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

#include "../AST/Listener/BaseListener.h"

class BashppListener;

#include "../bpp_include/bpp_codegen.h"
using code_segment = bpp::code_segment;
using bpp::generate_supershell_code;
using bpp::generate_delete_code;
using bpp::generate_method_call_code;
using bpp::generate_dynamic_cast_code;

#include "../bpp_include/bpp.h"

#include "../syntax_error.h"
#include "../internal_error.h"

#define skip_syntax_errors if (error_thrown) { \
		error_thrown = false; \
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
class BashppListener : public AST::BaseListener, std::enable_shared_from_this<BashppListener> {
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
		std::pair<uint16_t, uint16_t> target_bash_version = {5, 2};

		/**
		 * @var arguments
		 * @brief Command-line arguments to pass to the compiled program if run_on_exit is true
		 */
		std::vector<char*> arguments = {};

		std::shared_ptr<bpp::bpp_program> program = std::make_shared<bpp::bpp_program>();

		bool in_while_condition = false;
		std::shared_ptr<bpp::bash_while_condition> current_while_condition = nullptr;
		bool in_method = false;

		bool in_class = false;
		bool in_supershell = false;
		std::stack<std::monostate> bash_function_stack;
		bool should_declare_local() const;

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
		 * @brief This option is used by the language server.
		 * 
		 * The first element is the file path, and the second element is the contents of the file.
		 * 
		 * If set, it tells the listener not to read from the given file path, but instead to use the contents provided.
		 * 
		 * The language server uses this to parse unsaved changes in your editor.
		 */
		std::optional<std::pair<std::string, std::string>> replacement_file_contents;

		std::shared_ptr<bpp::bpp_class> primitive;

		bool error_thrown = false;

		bool program_has_errors = false;

		template <ASTNodePtrORToken T>
		void output_syntax_error(const T& error_ctx, const std::string& msg) {
			int line;
			int column;
			std::string text;

			if constexpr (ASTNodePtrType<T>) {
				line = static_cast<int>(error_ctx->getPosition().line);
				column = static_cast<int>(error_ctx->getPosition().column);
				// For the text:
				// Read from source_file the text corresponding to the node's position
				// I.e., between line/column and end_line/end_column

				std::ifstream infile(source_file);
				if (!infile.is_open()) {
					text = "<READ_ERROR>";
				} else {
					std::string line_content;
					uint32_t current_line = 0;
					while (std::getline(infile, line_content)) {
						if (current_line == static_cast<uint32_t>(line)) {
							// We're at the starting line
							if (line == error_ctx->getEndPosition().line) {
								// Single-line node
								text = line_content.substr(
									static_cast<size_t>(column),
									static_cast<size_t>(error_ctx->getEndPosition().column - column)
								);
							} else {
								// Multi-line node
								// Only report to the end of this line
								text = line_content.substr(static_cast<size_t>(column));
							}
						}
					}
				}
			} else if constexpr (ASTStringToken<T>) {
				line = static_cast<int>(error_ctx.getLine());
				column = static_cast<int>(error_ctx.getCharPositionInLine());
				text = error_ctx.getValue();
			} else if constexpr (ASTParameterToken<T>) {
				// Special case: Error reporting on a declared method parameter
				// FIXME(@rail5): Kind of hacky to handle special cases. Would prefer a general solution.
				line = static_cast<int>(error_ctx.getLine());
				column = static_cast<int>(error_ctx.getCharPositionInLine());
				if (error_ctx.type.has_value()) {
					text = "@" + error_ctx.type.value();
					if (error_ctx.pointer) text += "*";
					text += " " + error_ctx.name->getText();
				} else {
					text = error_ctx.name->getText();
				}
			}
			print_syntax_error_or_warning(source_file, line, column, text, msg, get_include_stack(), program);
			program_has_errors = true;
		}

		#define throw_syntax_error(token, msg) \
			output_syntax_error(token, msg); \
			error_thrown = true; /* Mark error_thrown so we know to skip the exit rule */ \
			node->clearChildren(); /* Skip traversing children */ \
			return;

		#define throw_syntax_error_from_exitRule(token, msg) \
			output_syntax_error(token, msg); \
			return;
		
		#define show_warning(token, msg) \
			if (!suppress_warnings) { \
				int line = static_cast<int>(token.getLine()); \
				int column = static_cast<int>(token.getCharPositionInLine()); \
				std::string text = token.getValue(); \
				print_syntax_error_or_warning(source_file, line, column, text, msg, get_include_stack(), program, true); \
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
	void set_target_bash_version(uint16_t major, uint16_t minor);
	void set_arguments(std::vector<char*> arguments);

	void set_replacement_file_contents(const std::string& file_path, const std::string& contents);

	std::shared_ptr<bpp::bpp_program> get_program();
	std::shared_ptr<std::set<std::string>> get_included_files();
	std::stack<std::string> get_include_stack();

	std::shared_ptr<bpp::bpp_code_entity> latest_code_entity();

	void enterProgram(std::shared_ptr<AST::Program> node) override;
	void exitProgram(std::shared_ptr<AST::Program> node) override;
	void enterArrayIndex(std::shared_ptr<AST::ArrayIndex> node) override;
	void exitArrayIndex(std::shared_ptr<AST::ArrayIndex> node) override;
	void enterBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node) override;
	void exitBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node) override;
	void enterBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node) override;
	void exitBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node) override;
	void enterBashArithmeticStatement(std::shared_ptr<AST::BashArithmeticStatement> node) override;
	void exitBashArithmeticStatement(std::shared_ptr<AST::BashArithmeticStatement> node) override;
	void enterBashCaseInput(std::shared_ptr<AST::BashCaseInput> node) override;
	void exitBashCaseInput(std::shared_ptr<AST::BashCaseInput> node) override;
	void enterBashCasePatternAction(std::shared_ptr<AST::BashCasePatternAction> node) override;
	void exitBashCasePatternAction(std::shared_ptr<AST::BashCasePatternAction> node) override;
	void enterBashCasePattern(std::shared_ptr<AST::BashCasePattern> node) override;
	void exitBashCasePattern(std::shared_ptr<AST::BashCasePattern> node) override;
	void enterBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node) override;
	void exitBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node) override;
	void enterBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node) override;
	void exitBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node) override;
	void enterBashCommand(std::shared_ptr<AST::BashCommand> node) override;
	void exitBashCommand(std::shared_ptr<AST::BashCommand> node) override;
	void enterBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) override;
	void exitBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) override;
	void enterBashForStatement(std::shared_ptr<AST::BashForStatement> node) override;
	void exitBashForStatement(std::shared_ptr<AST::BashForStatement> node) override;
	void enterBashIfCondition(std::shared_ptr<AST::BashIfCondition> node) override;
	void exitBashIfCondition(std::shared_ptr<AST::BashIfCondition> node) override;
	void enterBashIfElseBranch(std::shared_ptr<AST::BashIfElseBranch> node) override;
	void exitBashIfElseBranch(std::shared_ptr<AST::BashIfElseBranch> node) override;
	void enterBashIfRootBranch(std::shared_ptr<AST::BashIfRootBranch> node) override;
	void exitBashIfRootBranch(std::shared_ptr<AST::BashIfRootBranch> node) override;
	void enterBashIfStatement(std::shared_ptr<AST::BashIfStatement> node) override;
	void exitBashIfStatement(std::shared_ptr<AST::BashIfStatement> node) override;
	void enterBashInCondition(std::shared_ptr<AST::BashInCondition> node) override;
	void exitBashInCondition(std::shared_ptr<AST::BashInCondition> node) override;
	void enterBashPipeline(std::shared_ptr<AST::BashPipeline> node) override;
	void exitBashPipeline(std::shared_ptr<AST::BashPipeline> node) override;
	void enterBashRedirection(std::shared_ptr<AST::BashRedirection> node) override;
	void exitBashRedirection(std::shared_ptr<AST::BashRedirection> node) override;
	void enterBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node) override;
	void exitBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node) override;
	void enterBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node) override;
	void exitBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node) override;
	void enterBashVariable(std::shared_ptr<AST::BashVariable> node) override;
	void exitBashVariable(std::shared_ptr<AST::BashVariable> node) override;
	void enterBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node) override;
	void exitBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node) override;
	void enterBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node) override;
	void exitBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node) override;
	void enterBashFunction(std::shared_ptr<AST::BashFunction> node) override;
	void exitBashFunction(std::shared_ptr<AST::BashFunction> node) override;
	void enterBlock(std::shared_ptr<AST::Block> node) override;
	void exitBlock(std::shared_ptr<AST::Block> node) override;
	void enterClassDefinition(std::shared_ptr<AST::ClassDefinition> node) override;
	void exitClassDefinition(std::shared_ptr<AST::ClassDefinition> node) override;
	void enterConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node) override;
	void exitConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node) override;
	void enterDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) override;
	void exitDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) override;
	void enterDeleteStatement(std::shared_ptr<AST::DeleteStatement> node) override;
	void exitDeleteStatement(std::shared_ptr<AST::DeleteStatement> node) override;
	void enterDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node) override;
	void exitDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node) override;
	void enterDoublequotedString(std::shared_ptr<AST::DoublequotedString> node) override;
	void exitDoublequotedString(std::shared_ptr<AST::DoublequotedString> node) override;
	void enterDynamicCast(std::shared_ptr<AST::DynamicCast> node) override;
	void exitDynamicCast(std::shared_ptr<AST::DynamicCast> node) override;
	void enterDynamicCastTarget(std::shared_ptr<AST::DynamicCastTarget> node) override;
	void exitDynamicCastTarget(std::shared_ptr<AST::DynamicCastTarget> node) override;
	void enterHeredocBody(std::shared_ptr<AST::HeredocBody> node) override;
	void exitHeredocBody(std::shared_ptr<AST::HeredocBody> node) override;
	void enterHereString(std::shared_ptr<AST::HereString> node) override;
	void exitHereString(std::shared_ptr<AST::HereString> node) override;
	void enterIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) override;
	void exitIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) override;
	void enterMethodDefinition(std::shared_ptr<AST::MethodDefinition> node) override;
	void exitMethodDefinition(std::shared_ptr<AST::MethodDefinition> node) override;
	void enterNewStatement(std::shared_ptr<AST::NewStatement> node) override;
	void exitNewStatement(std::shared_ptr<AST::NewStatement> node) override;
	void enterObjectAssignment(std::shared_ptr<AST::ObjectAssignment> node) override;
	void exitObjectAssignment(std::shared_ptr<AST::ObjectAssignment> node) override;
	void enterObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node) override;
	void exitObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node) override;
	void enterObjectReference(std::shared_ptr<AST::ObjectReference> node) override;
	void exitObjectReference(std::shared_ptr<AST::ObjectReference> node) override;
	void enterParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node) override;
	void exitParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node) override;
	void enterPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node) override;
	void exitPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node) override;
	void enterPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) override;
	void exitPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) override;
	void enterProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) override;
	void exitProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) override;
	void enterRawSubshell(std::shared_ptr<AST::RawSubshell> node) override;
	void exitRawSubshell(std::shared_ptr<AST::RawSubshell> node) override;
	void enterRawText(std::shared_ptr<AST::RawText> node) override;
	void exitRawText(std::shared_ptr<AST::RawText> node) override;
	void enterRvalue(std::shared_ptr<AST::Rvalue> node) override;
	void exitRvalue(std::shared_ptr<AST::Rvalue> node) override;
	void enterSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) override;
	void exitSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) override;
	void enterSupershell(std::shared_ptr<AST::Supershell> node) override;
	void exitSupershell(std::shared_ptr<AST::Supershell> node) override;
	void enterTypeofExpression(std::shared_ptr<AST::TypeofExpression> node) override;
	void exitTypeofExpression(std::shared_ptr<AST::TypeofExpression> node) override;
	void enterValueAssignment(std::shared_ptr<AST::ValueAssignment> node) override;
	void exitValueAssignment(std::shared_ptr<AST::ValueAssignment> node) override;

};

#endif // SRC_LISTENER_BASHPPLISTENER_H_
