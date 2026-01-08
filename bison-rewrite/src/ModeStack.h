#pragma once

#include <stack>
#include <cassert>

class ModeStack {
	private:
		std::stack<int> modeStack;

		bool inSync() const;
	public:
		void push(int mode);
		void pop();
		int top() const;
		bool empty() const;
		size_t size() const;
};
