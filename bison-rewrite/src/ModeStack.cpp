#include "ModeStack.h"

extern void scanner_push_state(int state);
extern void scanner_pop_state();
extern int scanner_current_state();

bool ModeStack::inSync() const {
	if (modeStack.empty()) return true;
	return modeStack.top() == scanner_current_state();
}

void ModeStack::push(int mode) {
	assert(inSync() && "Mode stack out of sync with lexer");
	modeStack.push(mode);
	scanner_push_state(mode);
}

void ModeStack::pop() {
	if (!modeStack.empty()) {
		assert(inSync() && "Mode stack out of sync with lexer");
		modeStack.pop();
		scanner_pop_state();
	}
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
