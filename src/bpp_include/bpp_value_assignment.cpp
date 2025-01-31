/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_VALUE_ASSIGNMENT_CPP_
#define SRC_BPP_INCLUDE_BPP_VALUE_ASSIGNMENT_CPP_

#include "bpp.h"

namespace bpp {

bpp_value_assignment::bpp_value_assignment() {}

void bpp_value_assignment::set_nonprimitive_assignment(bool is_nonprimitive) {
	nonprimitive_assignment = is_nonprimitive;
}

void bpp_value_assignment::set_nonprimitive_object(std::shared_ptr<bpp_entity> object) {
	nonprimitive_object = object;
}

void bpp_value_assignment::set_lvalue_nonprimitive(bool is_nonprimitive) {
	lvalue_nonprimitive = is_nonprimitive;
}

void bpp_value_assignment::set_array_assignment(bool is_array) {
	array_assignment = is_array;
}

void bpp_value_assignment::set_adding(bool is_adding) {
	adding = is_adding;
}

bool bpp_value_assignment::is_nonprimitive_assignment() const {
	return nonprimitive_assignment;
}

std::shared_ptr<bpp_entity> bpp_value_assignment::get_nonprimitive_object() const {
	return nonprimitive_object;
}

bool bpp_value_assignment::lvalue_is_nonprimitive() const {
	return lvalue_nonprimitive;
}

bool bpp_value_assignment::is_array_assignment() const {
	return array_assignment;
}

bool bpp_value_assignment::is_adding() const {
	return adding;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_VALUE_ASSIGNMENT_CPP_
