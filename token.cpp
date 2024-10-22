#ifndef TOKEN_CPP_
#define TOKEN_CPP_
/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include "token.h"

bashpp::Token::Token() {}

bashpp::Token::Token(const std::string& token) : token(token) {}

bashpp::Token::Token(const std::string& token, TokenType type) : token(token), type(type) {}

#endif // TOKEN_CPP_