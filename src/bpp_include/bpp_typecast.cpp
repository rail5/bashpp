/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_TYPECAST_CPP_
#define SRC_BPP_INCLUDE_BPP_TYPECAST_CPP_

#include "bpp.h"

namespace bpp {

bpp_typecast::bpp_typecast() {}

void bpp_typecast::set_object_to_cast(std::shared_ptr<bpp_object> object) {
	this->object_to_cast = object;
}

std::shared_ptr<bpp_object> bpp_typecast::get_object_to_cast() const {
	return object_to_cast;
}

} // namespace bpp
 
#endif // SRC_BPP_INCLUDE_BPP_TYPECAST_CPP_
