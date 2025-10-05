/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

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
	return "${__this}__" + name;
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

} // namespace bpp
