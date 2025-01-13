/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_BPP_DESTRUCTOR_CPP_
#define ANTLR_BPP_BPP_DESTRUCTOR_CPP_

#include "bpp.h"

namespace bpp {

bpp_destructor::bpp_destructor(std::string name) : bpp_method(name) {}

bool bpp_destructor::add_parameter(bpp_method_parameter parameter) {
	return false;
}

} // namespace bpp

#endif // ANTLR_BPP_BPP_DESTRUCTOR_CPP_