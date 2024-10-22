#ifndef TOKENIZER_CPP_
#define TOKENIZER_CPP_
/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include "tokenizer.h"

namespace bashpp {

Tokenizer::Tokenizer() {}

const std::vector<std::string>& Tokenizer::getTokens() {
	return tokens;
}

void Tokenizer::parse(const std::string& script) {
	tokens.clear();
	std::string token = "";

	bool escaped = false;
	bool inString = false;
	bool inComment = false;

	for (char c : script) {
		switch (c) {
			case '\n':
				if (inComment) {
					inComment = false;
				}
				// Fall through to general whitespace handling below
			case ' ':
			case '\t':
			case '\f':
			case '\r':
			case '\v':
				if (!inString && !inComment && !escaped) {
					if (!token.empty()) {
						tokens.push_back(token);
						token.clear();
					}
				} else {
					if (!inComment)
						token += c;
				}
				break;
			case '\\':
				if (!escaped && !inComment) {
					escaped = true;
				} else if (!inComment) {
					token += c;
					escaped = false;
				}
				break;
			case '"':
				if (!escaped && !inComment) {
					inString = !inString;
				} else if (!inComment) {
					token += c;
					escaped = false;
				}
				break;
			case '#':
				if (!escaped && !inString) {
					inComment = true;
				} else if (!inComment) {
					token += c;
					escaped = false;
				}
				break;
			default:
				if (!inComment) {
					token += c;
					escaped = false;
				}
				break;
		}
	}
	if (!token.empty()) {
		tokens.push_back(token);
	}
}

Tokenizer::Tokenizer(const std::string& script) {
	parse(script);
}

} // namespace bashpp

#endif // TOKENIZER_CPP_