/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#ifndef NDEBUG
#include <iostream>
	// Number of spaces per indentation level when pretty-printing the AST or the entity tree
	#define PRETTYPRINT_INDENTATION_AMOUNT 4

	// Declare the prettyPrint function and operator<< in a base class (Entity & ASTNode)
	#define PRETTYPRINT_HELPERS(baseclassname) \
		virtual std::ostream& prettyPrint(std::ostream& os, std::size_t indentation_level = 0) const = 0; \
		friend std::ostream& operator<<(std::ostream& os, const baseclassname& node) { \
			return node.prettyPrint(os, 0); \
		}

	// Declare an override of prettyPrint in a derived class
	#define PRETTYPRINT_OVERRIDE(...) \
		std::ostream& prettyPrint(std::ostream& os, std::size_t indentation_level = 0) const override \
			__VA_ARGS__

	#define PRETTYPRINT_IMPLEMENTATION(classname, ...) \
		std::ostream& classname::prettyPrint(std::ostream& os, std::size_t indentation_level) const \
			__VA_ARGS__

	inline std::ostream& prettyprint_raw_code(std::ostream& os, const std::string& code) {
		for (const char c : code) {
			// Escape special characters for pretty-printing
			switch (c) {
				case '\n': os << "\\n"; break;
				case '\t': os << "\\t"; break;
				case '\r': os << "\\r"; break;
				default: os << c; break;
			}
		}
		return os;
	}
#else
	#define PRETTYPRINT_HELPERS(baseclassname)
	#define PRETTYPRINT_OVERRIDE(...)
	#define PRETTYPRINT_IMPLEMENTATION(classname, ...)
#endif // NDEBUG
