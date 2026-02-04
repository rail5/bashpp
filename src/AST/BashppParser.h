/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <memory>
#include <variant>
#include <stack>
#include <vector>
#include <AST/ASTNode.h>
#include <AST/Nodes/Nodes.h>
#include <error/ParserError.h>

typedef void* yyscan_t;

namespace AST {

/**
 * @class BashppParser
 * @brief A driver class to wrap around the Bison-generated parser for Bash++.
 * This class manages both the lexer and parser state, and provides methods to set the input source
 * 
 */
class BashppParser {
	private:
		yyscan_t lexer;
		std::shared_ptr<AST::Program> m_program = nullptr;
		bool current_command_can_receive_lvalues = true; // State variable needed by the parser

		bool utf16_mode = false; // Whether to use UTF-16 mode for character counting
		bool display_lexer_output = false;

		std::vector<ParserError> errors;

		std::string input_file_path = "<stdin>";
		std::vector<std::string> include_chain;
		
		enum class InputType {
			FILEPATH,
			FILEPTR,
			STRING_CONTENTS
		} input_type = InputType::FILEPATH;

		std::variant<std::string, FILE*, std::monostate> input_source = std::monostate{}; // Can be a file path, FILE*, or string contents
		std::string input_string_contents;

		FILE* input_file = nullptr;

		void _initialize_lexer();
		void _destroy_lexer();
		void _parse();
	public:
		BashppParser() = default;
		
		void setUTF16Mode(bool enabled);
		bool getUTF16Mode() const;

		void setDisplayLexerOutput(bool enabled);
		bool getDisplayLexerOutput() const;

		void setInputFromFilePath(const std::string& file_path);
		void setInputFromFilePtr(FILE* file_ptr, const std::string& file_path);
		void setInputFromStringContents(const std::string& contents);

		void setIncludeChain(const std::vector<std::string>& includes);

		std::shared_ptr<AST::Program> program();

		const std::vector<ParserError>& get_errors() const;
};

} // namespace AST
