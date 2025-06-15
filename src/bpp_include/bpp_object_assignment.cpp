/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"

namespace bpp {

bpp_object_assignment::bpp_object_assignment() {}

void bpp_object_assignment::set_lvalue(const std::string& lvalue) {
	this->lvalue = lvalue;
}

void bpp_object_assignment::set_rvalue(const std::string& rvalue) {
	this->rvalue = rvalue;
}

void bpp_object_assignment::set_lvalue_nonprimitive(bool is_nonprimitive) {
	lvalue_nonprimitive = is_nonprimitive;
}

void bpp_object_assignment::set_rvalue_nonprimitive(bool is_nonprimitive) {
	rvalue_nonprimitive = is_nonprimitive;
}

void bpp_object_assignment::set_lvalue_object(std::shared_ptr<bpp_entity> object) {
	lvalue_object = object;
}

void bpp_object_assignment::set_rvalue_object(std::shared_ptr<bpp_entity> object) {
	rvalue_object = object;
}

void bpp_object_assignment::set_adding(bool is_adding) {
	adding = is_adding;
}

void bpp_object_assignment::set_rvalue_array(bool is_array) {
	rvalue_array = is_array;
}

std::string bpp_object_assignment::get_lvalue() const {
	return lvalue;
}

std::string bpp_object_assignment::get_rvalue() const {
	return rvalue;
}

bool bpp_object_assignment::lvalue_is_nonprimitive() const {
	return lvalue_nonprimitive;
}

bool bpp_object_assignment::rvalue_is_nonprimitive() const {
	return rvalue_nonprimitive;
}

std::shared_ptr<bpp_entity> bpp_object_assignment::get_lvalue_object() const {
	return lvalue_object;
}

std::shared_ptr<bpp_entity> bpp_object_assignment::get_rvalue_object() const {
	return rvalue_object;
}

bool bpp_object_assignment::is_adding() const {
	return adding;
}

bool bpp_object_assignment::rvalue_is_array() const {
	return rvalue_array;
}

} // namespace bpp
