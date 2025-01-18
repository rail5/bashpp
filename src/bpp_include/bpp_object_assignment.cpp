/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_OBJECT_ASSIGNMENT_CPP_
#define SRC_BPP_INCLUDE_BPP_OBJECT_ASSIGNMENT_CPP_

#include "bpp.h"

namespace bpp {

bpp_object_assignment::bpp_object_assignment() {}

void bpp_object_assignment::set_lvalue(const std::string& lvalue) {
	this->lvalue = lvalue;
}

void bpp_object_assignment::set_rvalue(const std::string& rvalue) {
	this->rvalue = rvalue;
}

std::string bpp_object_assignment::get_lvalue() const {
	return lvalue;
}

std::string bpp_object_assignment::get_rvalue() const {
	return rvalue;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_OBJECT_ASSIGNMENT_CPP_