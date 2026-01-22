/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#pragma once

#include <stack>

/**
 * @struct ContextExpectations
 * @brief Represents the expectations for the current parsing context.
 *
 * can_take_primitive: Whether a primitive value is acceptable in the current context
 * can_take_object: Whether a non-primitive object is acceptable in the current
 *
 * In the event that a non-primitive object is referenced in a place where only a primitive is acceptable,
 * The Bash++ spec says that the .toPrimitive method should be called automatically.
 *
 * Further:
 * There are only four cases in which a non-primitive object can be used directly (without conversion to a primitive),
 * And there is only one case in which a primitive cannot be used directly.
 *
 * The four cases in which a non-primitive object can be used directly are:
 *
 *   1. In @delete statements
 *
 *   2. In non-primitive copies (rvalue) [the right-hand side of an assignment iff the left-hand was non-primitive]
 *
 *   3. In non-primitive copies (lvalue) [the left-hand side of an assignment, always]
 *
 *   4. When preceded by the '&' operator to get the object's address
 *
 * The one case in which a primitive cannot be used directly is as the rvalue in a non-primitive assignment.
 *  I.e., @nonPrimitiveObject="primitive value"
 *
 * In **all** other contexts: primitives are acceptable, and non-primitives are unacceptable.
 * 
 */
struct ContextExpectations {
	bool can_take_primitive = true;
	bool can_take_object = false;
};

class ExpectationsStack {
	private:
		std::stack<ContextExpectations> stack;
		static constexpr ContextExpectations default_expectations = {true, false};
	public:
		void push(const ContextExpectations& expectations) {
			stack.push(expectations);
		}

		void push(bool can_take_primitive, bool can_take_object) {
			stack.push({can_take_primitive, can_take_object});
		}

		void pop() {
			if (!stack.empty()) stack.pop();
		}

		ContextExpectations top() const {
			if (stack.empty()) return default_expectations;
			return stack.top();
		}

		bool canTakePrimitive() const {
			if (stack.empty()) return default_expectations.can_take_primitive;
			return stack.top().can_take_primitive;
		}

		bool canTakeObject() const {
			if (stack.empty()) return default_expectations.can_take_object;
			return stack.top().can_take_object;
		}
};
