/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "PointerDeclaration.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment PointerDeclaration::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(getPointer() != nullptr, "No pointer to declare");
	bpp_assert(getPointer()->isPointer(), "Object is not a pointer");
	bpp_assert(getPointer()->getType() != nullptr, "Pointer has no type");

	bpp::CodeGen::CodeSegment result;
	if (state->should_declare_local()) result.add_main_code("local ");
	result.add_main_code(getPointer()->getAddress());
	if (const auto* v = getInitialValue()) {
		result.egalitarian_merge(v->generateCode(state));
	} else {
		result.add_main_code("=0");
	}
	return result;
}

PRETTYPRINT_IMPLEMENTATION(PointerDeclaration, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	const auto* pointer = getPointer();
	bpp_assert(pointer != nullptr, "Pointer is null");
	const auto* type = pointer->getType();
	bpp_assert(type != nullptr, "Pointer has no type");
	os << indent << "(PointerDeclaration\n";
	os << indent << '@' << type->getName() << "* "
		<< pointer->getName();
	if (const auto* v = getInitialValue()) {
		os << '\n';
		v->prettyPrint(os, indentation_level + 1);
	} else {
		os << "=0";
	}
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
