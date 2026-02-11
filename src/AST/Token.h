/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <iostream>
#include <cstdint>

namespace AST {

/**
 * @class Token
 * @brief A class representing a token in the Bash++ AST.
 * Tokens store their value along with line and column information.
 * 
 * @tparam T The type of the token's value.
 */
template <class T>
class Token {
	private:
		T value;
		uint32_t line;
		uint32_t column;
	public:
		Token() = default;
		Token(const T& value, uint32_t line, uint32_t column) : value(value), line(line), column(column) {}
		const T& getValue() const {
			return value;
		}
		uint32_t getLine() const {
			return line;
		}
		uint32_t getCharPositionInLine() const {
			return column;
		}

		void setValue(const T& new_value) {
			value = new_value;
		}
		void setLine(uint32_t new_line) {
			line = new_line;
		}
		void setCharPositionInLine(uint32_t new_column) {
			column = new_column;
		}

		operator T() const {
			return value;
		}
		void operator=(const T& new_value) {
			value = new_value;
		}
		void operator+=(const T& append_value) {
			value += append_value;
		}
		bool operator==(const T& other) const {
			return value == other;
		}
		bool operator!=(const T& other) const {
			return value != other;
		}

		friend std::ostream& operator<<(std::ostream& os, const Token<T>& token) {
			os << token.value;
			return os;
		}
};

} // namespace AST
