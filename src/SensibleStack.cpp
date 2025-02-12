/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_SENSIBLESTACK_CPP_
#define SRC_SENSIBLESTACK_CPP_

#include <stack>
#include <type_traits>

/**
* A stack that returns zero if empty
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
* A stack that returns an empty string if empty
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

#endif // SRC_SENSIBLESTACK_CPP_
