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

void Tokenizer::parse(const std::string& single_line) {
	tokens.clear();
	std::string token = "";

	bool escaped = false;
	bool inString = false;

	for (char c : single_line) {
		switch (c) {
			case ' ':
			case '\t':
			case '\f':
			case '\r':
			case '\v':
			case '\n':
				if (!token.empty() && !inString && !escaped) {
					tokens.push_back(token);
					token = "";
				} else if (inString || escaped) {
					token += c;
					escaped = false;
				}
				break;
			case '#':
				if (!inString && !escaped) {
					if (!token.empty()) {
						tokens.push_back(token);
					}
					return;
				} else {
					token += c;
					escaped = false;
				}
				break;
			case '"':
				if (!escaped) {
					inString = !inString;
				} else {
					token += c;
					escaped = false;
				}
				break;
			case '\\':
				if (!escaped) {
					escaped = true;
				} else {
					token += c;
					escaped = false;
				}
				break;
			default:
				token += c;
				escaped = false;
				break;
		}
	}
	if (!token.empty()) {
		tokens.push_back(token);
	}
}

Tokenizer::Tokenizer(const std::string& single_line) {
	parse(single_line);
}

} // namespace bashpp

#endif // TOKENIZER_CPP_