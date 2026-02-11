/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "BashppParser.h"

#include <error/InternalError.h>
#include <error/ParserError.h>
#include <stdexcept>

struct LexerExtra;

extern int yylex_init(yyscan_t* scanner);
extern int yylex_destroy(yyscan_t scanner);
extern void yyset_in(FILE* in_str, yyscan_t scanner);

extern void initLexer(yyscan_t yyscanner);
extern void destroyLexer(yyscan_t yyscanner);

extern bool set_display_lexer_output(bool enable, yyscan_t yyscanner);
extern void set_utf16_mode(bool enable, yyscan_t yyscanner);

#include <flexbison/generated/parser.tab.hpp>
#include <flexbison/generated/lex.yy.hpp>

void AST::BashppParser::_initialize_lexer() {
	// If the input_source is empty, throw an exception
	if (std::holds_alternative<std::monostate>(input_source)) {
		throw bpp::ErrorHandling::InternalError("Attempted to initialize the lexer: Input source is not set");
	}

	if (yylex_init(&lexer) != 0) {
		throw bpp::ErrorHandling::InternalError("Could not initialize lexer");
	}

	switch (input_type) {
		case InputType::FILEPATH: {
			std::string file_path = std::get<std::string>(input_source);
			input_file = fopen(file_path.c_str(), "r");
			if (input_file == nullptr) {
				throw std::runtime_error("Could not open source file: " + file_path);
			}
			yyset_in(input_file, lexer);
			break;
		}
		case InputType::FILEPTR: {
			input_file = std::get<FILE*>(input_source);
			if (input_file == nullptr) {
				throw bpp::ErrorHandling::InternalError("Input FILE* is null");
			}
			yyset_in(input_file, lexer);
			break;
		}
		case InputType::STRING_CONTENTS: {
			input_string_contents = std::get<std::string>(input_source);
			// Create a temporary FILE* from the string contents
			input_file = fmemopen(reinterpret_cast<void*>(const_cast<char*>(input_string_contents.c_str())), input_string_contents.size(), "r");
			if (input_file == nullptr) {
				throw bpp::ErrorHandling::InternalError("Could not create FILE* from string contents");
			}
			yyset_in(input_file, lexer);
			break;
		}
	}

	initLexer(lexer);
	set_utf16_mode(utf16_mode, lexer);
	set_display_lexer_output(display_lexer_output, lexer);
}

void AST::BashppParser::_destroy_lexer() {
	destroyLexer(lexer);

	// Don't fclose() if:
	// a. We didn't **fopen()** (no FILE* is open)
	// b. **We** didn't fopen() (caller owns the FILE*)
	if (input_file != nullptr && input_type != InputType::FILEPTR) fclose(input_file);
	input_file = nullptr;
}

void AST::BashppParser::_parse() {
	_initialize_lexer();

	try {
		yy::parser parser(m_program,
			current_command_can_receive_lvalues,
			input_file_path,
			include_chain,
			errors,
			lexer);
		parser.parse(); // Returns an int, not needed by us
	} catch (...) {
		_destroy_lexer();
		throw;
	}

	_destroy_lexer();
}

void AST::BashppParser::setUTF16Mode(bool enabled) {
	utf16_mode = enabled;
}
bool AST::BashppParser::getUTF16Mode() const {
	return utf16_mode;
}

void AST::BashppParser::setDisplayLexerOutput(bool enabled) {
	display_lexer_output = enabled;
}
bool AST::BashppParser::getDisplayLexerOutput() const {
	return display_lexer_output;
}

void AST::BashppParser::setInputFromFilePath(const std::string& file_path) {
	input_type = InputType::FILEPATH;
	input_source = file_path;
	input_file_path = file_path;
}

void AST::BashppParser::setInputFromFilePtr(FILE* file_ptr, const std::string& file_path) {
	input_type = InputType::FILEPTR;
	input_source = file_ptr;
	input_file_path = file_path;
}

void AST::BashppParser::setInputFromStringContents(const std::string& contents) {
	input_type = InputType::STRING_CONTENTS;
	input_source = contents;
}

void AST::BashppParser::setIncludeChain(const std::vector<std::string>& includes) {
	include_chain = includes;
}

std::shared_ptr<AST::Program> AST::BashppParser::program() {
	if (m_program == nullptr) {
		_parse();
	}
	return m_program;
}

const std::vector<AST::ParserError>& AST::BashppParser::get_errors() const {
	return errors;
}
