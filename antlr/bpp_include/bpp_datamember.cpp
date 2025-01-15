/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_DATAMEMBER_CPP_
#define ANTLR_BPP_INCLUDE_BPP_DATAMEMBER_CPP_

#include "bpp.h"

namespace bpp {

bpp_datamember::bpp_datamember() {}

bpp_datamember::bpp_datamember(std::string name) : name(name) {}

void bpp_datamember::set_name(std::string name) {
	this->name = name;
}

void bpp_datamember::set_type(std::shared_ptr<bpp_class> type) {
	this->type = type;
}

void bpp_datamember::set_default_value(std::string default_value) {
	this->default_value = default_value;
}

void bpp_datamember::set_pre_access_code(std::string pre_access_code) {
	this->pre_access_code = pre_access_code;
}

void bpp_datamember::set_post_access_code(std::string post_access_code) {
	this->post_access_code = post_access_code;
}

void bpp_datamember::set_scope(bpp_scope scope) {
	this->scope = scope;
}

std::string bpp_datamember::get_name() const {
	return name;
}

std::shared_ptr<bpp_class> bpp_datamember::get_type() const {
	return type;
}

std::string bpp_datamember::get_default_value() const {
	// Sanitize default value before returning
	std::string sanitized = default_value;
	sanitized = replace_all(sanitized, "\\", "\\\\");
	sanitized = replace_all(sanitized, "\"", "\\\"");
	return sanitized;
}

std::string bpp_datamember::get_pre_access_code() const {
	return pre_access_code;
}

std::string bpp_datamember::get_post_access_code() const {
	return post_access_code;
}

bpp_scope bpp_datamember::get_scope() const {
	return scope;
}

void bpp_datamember::destroy() {
	name.clear();
	default_value.clear();
	scope = SCOPE_PRIVATE;
	type = nullptr;
}

} // namespace bpp

#endif // ANTLR_BPP_INCLUDE_BPP_DATAMEMBER_CPP
