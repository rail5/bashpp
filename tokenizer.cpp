#ifndef TOKENIZER_CPP_
#define TOKENIZER_CPP_
/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include "token.h"
#include "tokenizer.h"

namespace bashpp {

Tokenizer::Tokenizer() {}

const std::vector<bashpp::Token>& Tokenizer::getTokens() {
	return tokens;
}

void Tokenizer::parse(const std::string& script) {
	tokens.clear();
	std::string tokenString = "";
	bashpp::Token token;

	bool escaped = false;
	bool inString = false;
	bool inComment = false;

	for (char c : script) {
		switch (c) {
			// Separator handling
			case '\n':
				if (inComment) {
					inComment = false; // Newlines terminate comments, semicolons do not
				}
				// Fall through
			case ';':
				if (!inString && !inComment && !escaped) {
					if (!tokenString.empty()) {
						tokens.push_back(bashpp::Token(tokenString));
						tokenString.clear();
						// Push back a special token clarifying that this line has ended
						tokens.push_back(bashpp::Token(";", bashpp::TokenType::SPECIAL));
					}
				} else {
					if (!inComment)
						tokenString += c;
						escaped = false;
				}
				break;
			// General whitespace handling
			case ' ':
			case '\t':
			case '\f':
			case '\r':
			case '\v':
				if (!inString && !inComment && !escaped) {
					if (!tokenString.empty()) {
						tokens.push_back(bashpp::Token(tokenString));
						tokenString.clear();
					}
				} else {
					if (!inComment)
						tokenString += c;
						escaped = false;
				}
				break;
			// Escape sequences
			case '\\':
				if (!escaped && !inComment) {
					escaped = true;
				} else if (!inComment) {
					tokenString += c;
					escaped = false;
				}
				break;
			// Quotes
			case '"':
				if (!escaped && !inComment) {
					inString = !inString;
				} else if (!inComment) {
					tokenString += c;
					escaped = false;
				}
				break;
			// Comments
			case '#':
				if (!escaped && !inString) {
					inComment = true;
				} else if (!inComment) {
					tokenString += c;
					escaped = false;
				}
				break;
			// Directives to Bash++
			case '@':
				if (escaped) {
					tokenString += c;
					escaped = false;
				} else {
					// Push the @ symbol as its own token
					// What follows should be a directive to Bash++ (a keyword, object name, etc)
					if (!tokenString.empty()) {
						tokens.push_back(bashpp::Token(tokenString));
						tokenString.clear();
					}
					tokens.push_back(bashpp::Token("@", bashpp::TokenType::SPECIAL));
				}
				break;
			default:
				if (!inComment) {
					tokenString += c;
					escaped = false;
				}
				break;
		}
	}
	if (!tokenString.empty()) {
		tokens.push_back(bashpp::Token(tokenString));
	}
}

Tokenizer::Tokenizer(const std::string& script) {
	parse(script);
}

} // namespace bashpp

#endif // TOKENIZER_CPP_