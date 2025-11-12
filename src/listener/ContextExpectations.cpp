/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "ContextExpectations.h"

void ContextExpectationsStack::push(bool can_take_primitive, bool can_take_object)  {
	expectations_stack.push({can_take_primitive, can_take_object});
}

void ContextExpectationsStack::pop() {
	if (!expectations_stack.empty()) {
		expectations_stack.pop();
	}
}

ContextExpectations ContextExpectationsStack::top() const {
	if (expectations_stack.empty()) {
		return default_expectations;
	} else {
		return expectations_stack.top();
	}
}

bool ContextExpectationsStack::empty() const {
	return expectations_stack.empty();
}
