/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

 #ifndef SRC_BPP_INCLUDE_BPP_POINTER_DEREFERENCE_CPP_
 #define SRC_BPP_INCLUDE_BPP_POINTER_DEREFERENCE_CPP_
 
 #include "bpp.h"
 
namespace bpp {

bpp_pointer_dereference::bpp_pointer_dereference() {}

void bpp_pointer_dereference::set_value_assignment(std::shared_ptr<bpp::bpp_value_assignment> value_assignment) {
	this->value_assignment = value_assignment;
}

std::shared_ptr<bpp::bpp_value_assignment> bpp_pointer_dereference::get_value_assignment() const {
	return value_assignment;
}
 
} // namespace bpp
 
 #endif // SRC_BPP_INCLUDE_BPP_POINTER_DEREFERENCE_CPP_
