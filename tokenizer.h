#ifndef TOKENIZER_H_
#define TOKENIZER_H_

/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include <string>
#include <vector>

#include "token.h"

namespace bashpp {
class Tokenizer {
	private:
		std::vector<bashpp::Token> tokens = {};
	public:
		Tokenizer();
		explicit Tokenizer(const std::string& script);
		void parse(const std::string& script);
		const std::vector<bashpp::Token>& getTokens();
};
} // namespace bashpp

#endif // TOKENIZER_H_