/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_BPP_OBJECT_CPP_
#define ANTLR_BPP_BPP_OBJECT_CPP_

#include "bpp.h"

namespace bpp {

bpp_object::bpp_object(std::string name) : name(name) {}

bpp_object::bpp_object(std::string name, bool is_pointer) : name(name) {
	m_is_pointer = is_pointer;
	if (!m_is_pointer) {
		std::string type = object_class->get_name();
		address = "bpp__" + type + "__" + name;
	}
}

void bpp_object::set_class(std::shared_ptr<bpp_class> object_class) {
	this->object_class = object_class;
}

void bpp_object::set_pointer(bool is_pointer) {
	m_is_pointer = is_pointer;
}

void bpp_object::set_name(std::string name) {
	this->name = name;
	if (!m_is_pointer) {
		std::string type = object_class->get_name();
		address = "bpp__" + type + "__" + name;
	}
}

void bpp_object::set_address(std::string address) {
	this->address = address;
}

std::string bpp_object::get_name() const {
	return name;
}

std::string bpp_object::get_address() const {
	return address;
}

std::shared_ptr<bpp_class> bpp_object::get_class() const {
	return object_class;
}

bool bpp_object::is_pointer() const {
	return m_is_pointer;
}

} // namespace bpp

#endif // ANTLR_BPP_BPP_OBJECT_CPP_