/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_SENSIBLESTACK_H_
#define SRC_SENSIBLESTACK_H_

#include <stack>
#include <string>
#include <type_traits>

/**
* @class SensibleStack
*
* @brief A stack that returns zero if empty
*
* This is used heavily in the lexer
*/
template <typename T, typename = std::enable_if<std::is_arithmetic_v<T>>>
class SensibleStack : public std::stack<T> {
	public:
		SensibleStack() : std::stack<T>() {}
		static const T zero = T(0);

		T top() {
			if (this->empty()) {
				return zero;
			}
			return std::stack<T>::top();
		}

		inline void pop() {
			if (!this->empty()) {
				std::stack<T>::pop();
			}
		}
};

/**
* @class SensibleStringStack
*
* @brief A stack that returns an empty string if empty
*/
class SensibleStringStack : public std::stack<std::string> {
	public:
		SensibleStringStack() : std::stack<std::string>() {}

		std::string top() {
			if (this->empty()) {
				return "";
			}
			return std::stack<std::string>::top();
		}

		inline void pop() {
			if (!this->empty()) {
				std::stack<std::string>::pop();
			}
		}
};

#endif // SRC_SENSIBLESTACK_H_
