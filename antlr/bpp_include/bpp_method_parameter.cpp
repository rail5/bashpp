/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_METHOD_PARAMETER_CPP_
#define ANTLR_BPP_INCLUDE_BPP_METHOD_PARAMETER_CPP_

#include "bpp.h"

namespace bpp {

bpp_method_parameter::bpp_method_parameter(std::string name) : name(name) {}

void bpp_method_parameter::set_type(std::string type) {
	this->type = type;
}

std::string bpp_method_parameter::get_name() const {
	return name;
}

std::string bpp_method_parameter::get_type() const {
	return type;
}

}

#endif // ANTLR_BPP_INCLUDE_BPP_METHOD_PARAMETER_CPP
