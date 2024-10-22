#ifndef TOKEN_H_
#define TOKEN_H_
/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include <string>

namespace bashpp {

enum class TokenType {
	NORMAL,
	SPECIAL
};

struct Token {
		std::string token = "";
		TokenType type = TokenType::NORMAL;
		Token();
		explicit Token(const std::string& token);
		explicit Token(const std::string& token, TokenType type);

		// For debugging
		friend std::ostream& operator<<(std::ostream& os, const Token& token) {
			os << token.token;
			if (token.type == TokenType::SPECIAL) {
				os << " (SPECIAL)";
			}
			return os;
		}
};
} // namespace bashpp

#endif // TOKEN_H_