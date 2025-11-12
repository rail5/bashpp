/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <stack>
#include <utility>

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

/**
 * @class ContextExpectationsStack
 * @brief A stack to manage context expectations during parsing.
 * When we enter a parser rule which changes context expectations, we push the new expectations onto the stack
 * When we exit that rule, we pop the expectations off the stack, restoring the previous expectations.
 *
 * If the stack is empty, the default expectations are used (can_take_primitive = true, can_take_object = false).
 * See the description of ContextExpectations for more details and an explanation of *why* this is the default.
 * 
 */
class ContextExpectationsStack {
	private:
		std::stack<ContextExpectations> expectations_stack;
		static constexpr ContextExpectations default_expectations = {true, false};
	public:
		void push(bool can_take_primitive, bool can_take_object);
		void pop();

		ContextExpectations top() const;

		bool empty() const;
};
