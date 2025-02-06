/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_DATAMEMBER_CPP_
#define SRC_BPP_INCLUDE_BPP_DATAMEMBER_CPP_

#include "bpp.h"

namespace bpp {

bpp_datamember::bpp_datamember() {}

void bpp_datamember::set_default_value(const std::string& default_value) {
	this->default_value = default_value;
}

void bpp_datamember::set_scope(bpp_scope scope) {
	this->scope = scope;
}

void bpp_datamember::set_array(bool is_array) {
	array = is_array;
}

std::string bpp_datamember::get_address() const {
	return "bpp__" + type->get_name() + "__${__objectName}__" + name;
}

std::string bpp_datamember::get_default_value() const {
	return default_value;
}

bpp_scope bpp_datamember::get_scope() const {
	return scope;
}

bool bpp_datamember::is_array() const {
	return array;
}

void bpp_datamember::destroy() {
	name.clear();
	default_value.clear();
	scope = SCOPE_PRIVATE;
	type = nullptr;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_DATAMEMBER_CPP
