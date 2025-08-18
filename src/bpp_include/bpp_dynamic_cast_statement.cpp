/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"

namespace bpp {

bpp_dynamic_cast_statement::bpp_dynamic_cast_statement() {}

void bpp_dynamic_cast_statement::set_cast_to(const std::string& cast_to) {
	this->cast_to = cast_to;
}

std::string bpp_dynamic_cast_statement::get_cast_to() const {
	return cast_to;
}

} // namespace bpp
