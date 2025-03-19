/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_METHOD_CPP_
#define SRC_BPP_INCLUDE_BPP_METHOD_CPP_

#include "bpp.h"

namespace bpp {

bpp_method::bpp_method() {}

bpp_method::bpp_method(const std::string& name) : name(name) {}

/**
 * @brief Add a non-primitive object as a parameter to the method
 * 
 * This function ensures that code within the scope of the method is able to access an object
 * 	passed as an argument to the method, by adding the object to the local_objects map.
 */
bool bpp_method::add_object_as_parameter(std::shared_ptr<bpp_object> object) {
	std::string name = object->get_name();
	if (objects.find(name) != objects.end() || local_objects.find(name) != local_objects.end()) {
		return false;
	}

	// Verify that the type of the object is a valid class
	std::string type = object->get_class()->get_name();
	if (classes.find(type) == classes.end()) {
		return false;
	}
	object->set_pointer(true);
	local_objects[name] = object;

	return true;
}

/**
 * @brief Add a parameter to the method
 * 
 * This function adds a parameter to the method
 */
bool bpp_method::add_parameter(std::shared_ptr<bpp_method_parameter> parameter) {
	std::string name = parameter->get_name();
	for (auto& p : parameters) {
		if (p->get_name() == name) {
			return false;
		}
	}

	if (parameter->get_type()->get_name() != "primitive") {
		// Instantiate a temporary object for the parameter
		std::shared_ptr<bpp_object> object = std::make_shared<bpp_object>(name, true);
		object->set_class(parameter->get_type());
		object->set_name(name);
		object->set_address(name);
		if (!add_object_as_parameter(object)) {
			return false;
		}
	}

	parameters.push_back(parameter);
	return true;
}

void bpp_method::set_name(const std::string& name) {
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

/**
 * @brief Destruct all local objects
 * 
 * This function destructs all local objects in the method by calling their destructors and deleting them.
 * 
 * This is called as we exit the method, to ensure that all local objects are cleaned up.
 */
void bpp_method::destruct_local_objects() {
	for (auto& o : local_objects) {
		// If it's a pointer, don't delete it
		if (o.second->is_pointer()) {
			continue;
		}
		// If it has a destructor, call it
		if (o.second->get_class()->has_destructor()) {
			*code << "bpp__" + o.second->get_class()->get_name() + "____destructor " + o.second->get_address() + "\n" << std::flush;
		}
		// Call delete
		*code << "bpp__" + o.second->get_class()->get_name() + "____delete " + o.second->get_address() + "\n" << std::flush;
	}
	local_objects.clear();
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_METHOD_CPP
