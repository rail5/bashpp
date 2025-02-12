/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_DESTRUCTOR_CPP_
#define SRC_BPP_INCLUDE_BPP_DESTRUCTOR_CPP_

#include "bpp.h"

namespace bpp {

bpp_destructor::bpp_destructor() : bpp_method("destructor") {}

bpp_destructor::bpp_destructor(const std::string& name) : bpp_method(name) {}

bool bpp_destructor::add_parameter(std::shared_ptr<bpp_method_parameter> parameter) {
	return false;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_DESTRUCTOR_CPP
