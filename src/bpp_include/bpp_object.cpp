/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_OBJECT_CPP_
#define SRC_BPP_INCLUDE_BPP_OBJECT_CPP_

#include "bpp.h"

namespace bpp {

bpp_object::bpp_object() {}

bpp_object::bpp_object(const std::string& name) : name(name) {}

bpp_object::bpp_object(const std::string& name, bool is_pointer) : name(name), m_is_pointer(is_pointer) {}

void bpp_object::set_class(std::shared_ptr<bpp_class> object_class) {
	this->type = object_class;
}

void bpp_object::set_pointer(bool is_pointer) {
	m_is_pointer = is_pointer;
}

void bpp_object::set_name(const std::string& name) {
	this->name = name;
	if (!m_is_pointer) {
		std::string type = this->type->get_name();
		address = "bpp__" + type + "__" + name;
	}
}

void bpp_object::set_address(const std::string& address) {
	this->address = address;
}

void bpp_object::set_assignment_value(const std::string& assignment_value) {
	this->assignment_value = assignment_value;
}

void bpp_object::set_pre_access_code(const std::string& pre_access_code) {
	this->pre_access_code = pre_access_code;
}

void bpp_object::set_post_access_code(const std::string& post_access_code) {
	this->post_access_code = post_access_code;
}

void bpp_object::set_nullptr() {
	if (m_is_pointer) {
		assignment_value = bpp::bpp_nullptr;
	}
}

std::string bpp_object::get_name() const {
	return name;
}

std::string bpp_object::get_address() const {
	return address;
}

std::string bpp_object::get_assignment_value() const {
	return assignment_value;
}

std::shared_ptr<bpp_class> bpp_object::get_class() const {
	return type;
}

std::string bpp_object::get_pre_access_code() const {
	return pre_access_code;
}

std::string bpp_object::get_post_access_code() const {
	return post_access_code;
}

bool bpp_object::is_nullptr() const {
	return m_is_pointer && assignment_value == bpp::bpp_nullptr;
}

bool bpp_object::is_pointer() const {
	return m_is_pointer;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_OBJECT_CPP
