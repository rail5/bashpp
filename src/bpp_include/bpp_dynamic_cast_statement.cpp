/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_DYNAMIC_CAST_STATEMENT_CPP_
#define SRC_BPP_INCLUDE_BPP_DYNAMIC_CAST_STATEMENT_CPP_

#include "bpp.h"

namespace bpp {

bpp_dynamic_cast_statement::bpp_dynamic_cast_statement() {}

void bpp_dynamic_cast_statement::set_cast_to(std::shared_ptr<bpp::bpp_class> cast_to) {
	this->cast_to = cast_to;
}

std::shared_ptr<bpp::bpp_class> bpp_dynamic_cast_statement::get_cast_to() const {
	return cast_to;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_DYNAMIC_CAST_STATEMENT_CPP_
