/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BashppParser.h"

#include "../internal_error.h"
#include <stdexcept>

struct LexerExtra;

extern int yylex_init(yyscan_t* scanner);
extern int yylex_destroy(yyscan_t scanner);
extern void yyset_in(FILE* in_str, yyscan_t scanner);

extern void initLexer(yyscan_t yyscanner);
extern void destroyLexer(yyscan_t yyscanner);

extern bool set_display_lexer_output(bool enable, yyscan_t yyscanner);
extern void set_utf16_mode(bool enable, yyscan_t yyscanner);

#include "../flexbison/parser.tab.hpp"
#include "../flexbison/lex.yy.hpp"

void AST::BashppParser::_initialize_lexer() {
	// If the input_source is empty, throw an exception
	if (std::holds_alternative<std::monostate>(input_source)) {
		throw internal_error("Attempted to initialize the lexer: Input source is not set");
	}

	if (yylex_init(&lexer) != 0) {
		throw internal_error("Could not initialize lexer");
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
				throw internal_error("Input FILE* is null");
			}
			yyset_in(input_file, lexer);
			break;
		}
		case InputType::STRING_CONTENTS: {
			std::string contents = std::get<std::string>(input_source);
			// Create a temporary FILE* from the string contents
			input_file = fmemopen(reinterpret_cast<void*>(const_cast<char*>(contents.c_str())), contents.size(), "r");
			if (input_file == nullptr) {
				throw internal_error("Could not create FILE* from string contents");
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

	if (input_file != nullptr) fclose(input_file);
	input_file = nullptr;
}

void AST::BashppParser::_parse() {
	_initialize_lexer();

	try {
		yy::parser parser(m_program,
			current_command_can_receive_lvalues,
			input_file_path,
			include_chain,
			lexer);
		int parse_result = parser.parse();
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

void AST::BashppParser::setIncludeChain(const std::stack<std::string>& includes) {
	include_chain = includes;
}

std::shared_ptr<AST::Program> AST::BashppParser::program() {
	if (m_program == nullptr) {
		_parse();
	}
	return m_program;
}
