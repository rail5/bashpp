/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include <IR/entities/Object.h>
#include <IR/entities/Method.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Program.h>
#include "ObjectReference.h"

#include <error/InternalError.h>

namespace bpp::IR {

namespace {

std::string get_encased_reference(const std::string& ref, std::uint8_t indirection_level) {
	std::string encase_open, encase_close, indirection;
	switch (indirection_level) {
		case 2:
			indirection = "!";
			[[ fallthrough ]];
		case 1:
			encase_open = "${";
			encase_close = "}";
			[[ fallthrough ]];
		case 0:
		default:
			// No indirection, no encasing
			break;
	}
	return encase_open + indirection + ref + encase_close;
}

#ifndef NDEBUG
std::string get_reference_chain_prettyprint_string(const ObjectReference::ReferenceChain& chain) {
	// E.g.: @object.inner.member
	std::string result = "@";

	bpp_assert(!chain.empty(), "Reference chain is empty in get_reference_chain_prettyprint_string()");
	const auto root = chain.getRoot().lock();
	bpp_assert(root != nullptr, "Root object in reference chain is null in get_reference_chain_prettyprint_string()");
	result += root->getName();
	for (auto it = std::next(chain.begin()); it != chain.end(); ++it) {
		auto obj = (*it).lock();
		bpp_assert(obj != nullptr, "Data member in reference chain is null in get_reference_chain_prettyprint_string()");
		result += '.' + obj->getName();
	}

	if (chain.hasMethod()) {
		auto method = chain.getMethod().lock();
		bpp_assert(method != nullptr, "Method in reference chain is null in get_reference_chain_prettyprint_string()");
		result += '.' + method->getName();
	}

	return result;
}
#endif // NDEBUG

} // anonymous namespace

bpp::CodeGen::CodeSegment ObjectReference::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "ObjectReference::generate_code() should be called with a non-null state pointer");
	bpp_assert(!getReferenceChain().empty(), "ObjectReference::generate_code() should be called with a non-empty reference chain");
	bpp_assert(!getReferenceChain().getRoot().expired(), "ObjectReference::generate_code() should be called with a non-null object pointer");
	bpp::CodeGen::CodeSegment result;

	/* The purpose of ObjectReference::generate_code is to calculate the address of the final object in the reference chain
	 * In order to do this:
	 * 1. Get the address of the root object                                   bpp__Type__name
	 *    If the chain is empty, return the address of the root object
	 * 2. Append the address of the first data member                          bpp__Type__name__member
	 *    If there are no more data members, return this concatenated address
	 * 3. For each subsequent data member:
	 *    a. Dereference the current address
	 *    b. Append the data member's address                                  tmp=${bpp__Type__name__member}__member2
	 *                                                                         tmp2=${!tmp}__member3
	 *                                                                         tmp3=${!tmp2}__member4
	 *                                                                         etc
	 *
	 * If the root object is a pointer, however, we need to dereference it before appending the first data member's address.
	 *                                                                         tmp=${bpp____ptr__Type__name}__member
	 *                                                                         tmp2=$(!tmp)__member2
	 *                                                                         etc
	 */
	const auto& ref = getReferenceChain();
	const auto root = ref.getRoot().lock();

	std::string current_address = root->getAddress();
	std::uint8_t indirection_level = root->isPointer() ? 1 : 0;

	for (auto it = std::next(ref.begin()); it != ref.end(); ++it) {
		const auto dm = (*it).lock();
		bpp_assert(dm != nullptr, "Data member in reference chain is null in ObjectReference::generate_code()");

		if (indirection_level > 0) {
			// If there's been indirection, we need to set up temporaries & dereference
			const std::string lhs = current_address + dm->getAddress();
			const std::string rhs = get_encased_reference(current_address, indirection_level) + dm->getAddress();

			if (state->should_declare_local()) result.add_pre_code("local ");

			std::string assignment = lhs + "=";
			assignment += rhs + "\n";
			result.add_pre_code(std::move(assignment)); // bpp__Type__name__member...=${!bpp__Type__name}__member...

			result.add_post_code("\nunset " + lhs); // TODO(@rail5): Do we just forego the unset if we're local?
			// Maybe in certain "safer" cases, e.g. small non-recursive functions etc
		}

		current_address += dm->getAddress(); // Append suffix without any encasement, repeatedly, until the end of the sequence
		indirection_level = std::min(indirection_level + 1, 2);
	}

	if (ref.hasMethod()) {
		auto method = ref.getMethod().lock();
		bpp_assert(method != nullptr, "Method in reference chain is null in ObjectReference::generate_code()");
		result.add_main_code(method->getAddress() + " "); // Call to the method
	}

	// Add the final address of the object, with appropriate encasement for any indirection
	result.add_main_code(get_encased_reference(current_address, indirection_level > 0 ? indirection_level - 1 : 0));

	return result;
}

PRETTYPRINT_IMPLEMENTATION(ObjectReference, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ObjectReference "
		<< get_reference_chain_prettyprint_string(getReferenceChain())
		<< ")\n";
	return os;
})

} // namespace bpp::IR
