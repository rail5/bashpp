/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DeleteStatement.h"

#include <error/InternalError.h>

namespace bpp::IR {

void DeleteStatement::setObjectToDelete(std::shared_ptr<ObjectReference> obj_ref) {
	bpp_assert(obj_ref != nullptr, "Object reference is null");
	bpp_assert(obj_ref->isPointer() || obj_ref->isNonprimitive(), "Object reference is neither a pointer nor a nonprimitive");

	destructor_method_call = std::make_shared<ObjectReference>(*obj_ref);
	destructor_method_call->addMethodCall_UNSAFE("__destructor");
	destructor_method_call->setLvalue(true);

	delete_method_call = std::make_shared<ObjectReference>(*obj_ref);
	delete_method_call->addMethodCall_UNSAFE("__delete");
	delete_method_call->setLvalue(true);
}

bpp::CodeGen::CodeSegment DeleteStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(destructor_method_call != nullptr, "Destructor method call is null");
	bpp_assert(delete_method_call != nullptr, "Delete method call is null");

	bpp::CodeGen::CodeSegment result;

	result.absorb_all_to_main(destructor_method_call->generateCode(state));
	result.add_main_code("\n");
	result.absorb_all_to_main(delete_method_call->generateCode(state));
	result.add_main_code("\n");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(DeleteStatement, {
	bpp_assert(destructor_method_call != nullptr, "Destructor method call is null");
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(DeleteStatement @delete \n";
	ObjectReference::ReferenceChain reference_chain = destructor_method_call->getReferenceChain();
	reference_chain.removeMethod();
	os << get_reference_chain_prettyprint_string(reference_chain)
		<< ")\n";
	return os;
});

} // namespace bpp::IR
