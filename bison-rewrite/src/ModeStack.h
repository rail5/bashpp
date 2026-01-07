#pragma once

#include <stack>
#include <cassert>

class ModeStack {
	private:
		std::stack<int> modeStack;

		bool inSync();
	public:
		void push(int mode);

		void pop();

		int top();
};
