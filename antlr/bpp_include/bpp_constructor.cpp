/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_CONSTRUCTOR_CPP
#define ANTLR_BPP_INCLUDE_BPP_CONSTRUCTOR_CPP

#include "bpp.h"

namespace bpp {

bpp_constructor::bpp_constructor(std::string name) : bpp_method(name) {}

bool bpp_constructor::add_parameter(bpp_method_parameter parameter) {
	return false;
}

} // namespace bpp

#endif // ANTLR_BPP_INCLUDE_BPP_CONSTRUCTOR_CP
