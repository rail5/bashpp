/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_DELETE_STATEMENT_CPP_
#define SRC_BPP_INCLUDE_BPP_DELETE_STATEMENT_CPP_

#include "bpp.h"

namespace bpp {

void bpp_delete_statement::set_object_to_delete(std::shared_ptr<bpp_object> object) {
	this->object_to_delete = object;
}

void bpp_delete_statement::set_force_pointer(bool force_pointer) {
	this->force_ptr = force_pointer;
}

std::shared_ptr<bpp_object> bpp_delete_statement::get_object_to_delete() const {
	return object_to_delete;
}

bool bpp_delete_statement::force_pointer() const {
	return force_ptr;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_DELETE_STATEMENT_CPP_
