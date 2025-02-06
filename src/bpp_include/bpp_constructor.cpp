/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CONSTRUCTOR_CPP
#define SRC_BPP_INCLUDE_BPP_CONSTRUCTOR_CPP

#include "bpp.h"

namespace bpp {

bpp_constructor::bpp_constructor() : bpp_method("constructor") {}

bpp_constructor::bpp_constructor(const std::string& name) : bpp_method(name) {}

bool bpp_constructor::add_parameter(std::shared_ptr<bpp_method_parameter> parameter) {
	return false;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CONSTRUCTOR_CP
