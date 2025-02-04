/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_METHOD_CPP_
#define SRC_BPP_INCLUDE_BPP_METHOD_CPP_

#include "bpp.h"

namespace bpp {

bpp_method::bpp_method() {}

bpp_method::bpp_method(std::string name) : name(name) {}

bool bpp_method::add_object(std::shared_ptr<bpp_object> object) {
	std::string name = object->get_name();
	if (objects.find(name) != objects.end() || local_objects.find(name) != local_objects.end()) {
		return false;
	}

	// Verify that the type of the object is a valid class
	std::string type = object->get_class()->get_name();
	if (classes.find(type) == classes.end()) {
		return false;
	}

	local_objects[name] = object;

	// Add the code for the object
	std::string object_code = "";

	// Is it a pointer?
	if (object->is_pointer()) {
		object_code += object->get_address() + "=\"" + object->get_assignment_value() + "\"\n";
	} else {
		object_code += "bpp__" + type + "____new " + name + "\n";
	}

	// Call the constructor if it exists
	if (object->get_class()->has_constructor()) {
		object_code += "bpp__" + type + "____constructor " + name + " " + (object->is_pointer() ? "1" : "0") + "\n";
	}

	code += object_code;
	return true;
}

bool bpp_method::add_parameter(std::shared_ptr<bpp_method_parameter> parameter) {
	std::string name = parameter->get_name();
	for (auto& p : parameters) {
		if (p->get_name() == name) {
			return false;
		}
	}
	parameters.push_back(parameter);
	return true;
}

void bpp_method::set_name(std::string name) {
	this->name = name;
}

void bpp_method::set_scope(bpp_scope scope) {
	this->scope = scope;
}

void bpp_method::set_virtual(bool is_virtual) {
	m_is_virtual = is_virtual;
}

void bpp_method::set_inherited(bool is_inherited) {
	inherited = is_inherited;
}

std::string bpp_method::get_name() const {
	return name;
}

std::vector<std::shared_ptr<bpp_method_parameter>> bpp_method::get_parameters() const {
	return parameters;
}

bpp_scope bpp_method::get_scope() const {
	return scope;
}

bool bpp_method::is_virtual() const {
	return m_is_virtual;
}

bool bpp_method::is_inherited() const {
	return inherited;
}

void bpp_method::destruct_local_objects() {
	for (auto& o : local_objects) {
		// If it has a destructor, call it
		if (o.second->get_class()->has_destructor()) {
			code += "bpp__" + o.second->get_class()->get_name() + "____destructor " + o.first + " 0\n";
		}
		// Call delete
		code += "bpp__" + o.second->get_class()->get_name() + "____delete " + o.first + " 0\n";
	}
	local_objects.clear();
}

void bpp_method::destroy() {
	name.clear();
	parameters.clear();
	code.clear();
	nextline_buffer.clear();
	postline_buffer.clear();
	scope = SCOPE_PRIVATE;
	m_is_virtual = false;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_METHOD_CPP
