/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_OBJECT_REFERENCE_CPP_
#define SRC_BPP_INCLUDE_BPP_OBJECT_REFERENCE_CPP_

#include "bpp.h"

namespace bpp {

bpp_object_reference::bpp_object_reference() {}

void bpp_object_reference::set_reference_type(bpp::reference_type reference_type) {
	this->reference_type = reference_type;
}

void bpp_object_reference::set_array_index(const std::string& array_index) {
	this->array_index = array_index;
}

bpp::reference_type bpp_object_reference::get_reference_type() const {
	return reference_type;
}

std::string bpp_object_reference::get_array_index() const {
	return array_index;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_OBJECT_REFERENCE_CPP_
