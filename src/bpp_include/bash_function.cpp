/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BASH_FUNCTION_CPP_
#define SRC_BPP_INCLUDE_BASH_FUNCTION_CPP_

#include "bpp.h"

namespace bpp {

bash_function::bash_function() {}

void bash_function::set_name(const std::string& name) {
	this->name = name;
}

std::string bash_function::get_name() const {
	return name;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_FUNCTION_CPP_
