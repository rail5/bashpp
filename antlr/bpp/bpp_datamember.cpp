/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_BPP_DATAMEMBER_CPP_
#define ANTLR_BPP_BPP_DATAMEMBER_CPP_

#include "bpp.h"

namespace bpp {

bpp_datamember::bpp_datamember(std::string name) : name(name) {}

void bpp_datamember::set_type(std::string type) {
	this->type = type;
}

void bpp_datamember::set_default_value(std::string default_value) {
	this->default_value = default_value;
}

void bpp_datamember::set_scope(bpp_scope scope) {
	this->scope = scope;
}

std::string bpp_datamember::get_name() const {
	return name;
}

std::string bpp_datamember::get_type() const {
	return type;
}

std::string bpp_datamember::get_default_value() const {
	return default_value;
}

bpp_scope bpp_datamember::get_scope() const {
	return scope;
}

} // namespace bpp

#endif // ANTLR_BPP_BPP_DATAMEMBER_CPP_