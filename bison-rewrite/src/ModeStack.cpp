/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "ModeStack.h"

extern void scanner_push_state(int state, yyscan_t yyscanner);
extern void scanner_pop_state(yyscan_t yyscanner);
extern int scanner_current_state(yyscan_t yyscanner);

void ModeStack::bind(yyscan_t lexer) {
	if (relevant_lexer != nullptr) {
		return; // Already bound
	}
	relevant_lexer = lexer;
}

bool ModeStack::inSync() const {
	if (relevant_lexer == nullptr) return false;
	if (modeStack.empty()) return true;
	return modeStack.top() == scanner_current_state(relevant_lexer);
}

void ModeStack::push(int mode) {
	assert(inSync() && "Mode stack out of sync with lexer");
	modeStack.push(mode);
	scanner_push_state(mode, relevant_lexer);
}

void ModeStack::pop() {
	if (!modeStack.empty()) {
		assert(inSync() && "Mode stack out of sync with lexer");
		modeStack.pop();
		scanner_pop_state(relevant_lexer);
	}
}

void ModeStack::clear() {
	while (!modeStack.empty()) {
		pop();
	}
	relevant_lexer = nullptr;
}

int ModeStack::top() const {
	if (!modeStack.empty()) {
		assert(inSync() && "Mode stack out of sync with lexer");
		return modeStack.top();
	}
	return 0; // Default: INITIAL
			// CAREFUL. This depends on generated code from Flex
			// We're relying on Flex doing #define INITIAL 0
}

bool ModeStack::empty() const {
	return modeStack.empty();
}

size_t ModeStack::size() const {
	return modeStack.size();
}
