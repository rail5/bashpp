#pragma once

#include <stack>
#include <cassert>

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif


class ModeStack {
	private:
		std::stack<int> modeStack;
		yyscan_t relevant_lexer;

		bool inSync() const;
	public:
		void bind(yyscan_t lexer);
		void push(int mode);
		void pop();
		void clear();
		int top() const;
		bool empty() const;
		size_t size() const;
};
