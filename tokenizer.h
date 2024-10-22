#ifndef TOKENIZER_H_
#define TOKENIZER_H_

/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include <string>
#include <vector>

namespace bashpp {
class Tokenizer {
	private:
		std::vector<std::string> tokens = {};
	public:
		Tokenizer();
		explicit Tokenizer(const std::string& script);
		void parse(const std::string& script);
		const std::vector<std::string>& getTokens();
};
} // namespace bashpp

#endif // TOKENIZER_H_