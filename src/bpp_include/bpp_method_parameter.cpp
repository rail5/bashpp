/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_METHOD_PARAMETER_CPP_
#define SRC_BPP_INCLUDE_BPP_METHOD_PARAMETER_CPP_

#include "bpp.h"

namespace bpp {

bpp_method_parameter::bpp_method_parameter(const std::string& name) : name(name) {}

void bpp_method_parameter::set_type(std::shared_ptr<bpp_class> type) {
	this->type = type;
}

std::string bpp_method_parameter::get_name() const {
	return name;
}

std::shared_ptr<bpp_class> bpp_method_parameter::get_type() const {
	return type;
}

}

#endif // SRC_BPP_INCLUDE_BPP_METHOD_PARAMETER_CPP
