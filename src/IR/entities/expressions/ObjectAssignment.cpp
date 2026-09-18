/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ObjectAssignment.h"

#include <IR/entities/expressions/ObjectReference.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment ObjectAssignment::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(lhs != nullptr, "LHS is null");
	bpp_assert(rhs != nullptr, "RHS is null");

	bpp::CodeGen::CodeSegment result;

	const bool is_nonprimitive_copy = lhs->isNonprimitive() && rhs->isRvalueNonprimitive();

	// FIXME(@rail5): HACK.
	if (is_nonprimitive_copy) {
		ObjectReference::ReferenceChain lhs_chain = lhs->getReferenceChain();
		auto copy_method = lhs_chain.getFinalObject().lock()->getType().lock()->getMethod_UNSAFE("__copy");
		lhs_chain.setMethod(copy_method);

		ObjectReference copy_call;
		copy_call.inherit(shared_from_this());
		copy_call.setReferenceChain(std::move(lhs_chain));
		copy_call.setLvalue(true);

		ObjectReference rhs_ref = *rhs->getRvalueReference();
		rhs_ref.setAddressOf(true);

		result.egalitarian_merge(copy_call.generateCode(state));
		result.add_main_code(" ");
		result.egalitarian_merge(rhs_ref.generateCode(state));
		result.add_main_code("\n");
		return result;
	}

	if (state->should_declare_local()) result.add_main_code("local ");
	result.add_main_code("__assignment");
	result.egalitarian_merge(rhs->generateCode(state));
	result.add_main_code("\n");

	if (!state->should_declare_local()) result.add_post_code("\nunset __assignment\n");

	result.add_main_code("printf -v \"");
	result.egalitarian_merge(lhs->generateCode(state));
	result.add_main_code("\" '%s' \"$__assignment\"\n");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(ObjectAssignment, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ObjectAssignment\n";
	lhs->prettyPrint(os, indentation_level + 1);
	rhs->prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
