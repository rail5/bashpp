/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#ifndef NDEBUG
	// Number of spaces per indentation level when pretty-printing the AST or the entity tree
	#define PRETTYPRINT_INDENTATION_AMOUNT 4

	// Declare the prettyPrint function and operator<< in a base class (Entity & ASTNode)
	#define PRETTYPRINT_HELPERS(baseclassname) \
		virtual std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const = 0; \
		friend std::ostream& operator<<(std::ostream& os, const baseclassname& node) { \
			return node.prettyPrint(os, 0); \
		}
	
	// Declare an override of prettyPrint in a derived class
	#define PRETTYPRINT_OVERRIDE \
		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override
	
	#define PRETTYPRINT_IMPLEMENTATION(classname, ...) \
		std::ostream& classname::prettyPrint(std::ostream& os, size_t indentation_level) const \
			__VA_ARGS__
	
	#define PRETTYPRINT_IMPLEMENTATION_IN_HEADER(...) \
		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override \
			__VA_ARGS__
#else
	#define PRETTYPRINT_HELPERS(baseclassname)
	#define PRETTYPRINT_OVERRIDE
	#define PRETTYPRINT_IMPLEMENTATION(classname, ...)
	#define PRETTYPRINT_IMPLEMENTATION_IN_HEADER(...)
#endif // NDEBUG
