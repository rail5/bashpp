/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"
#include "bpp_codegen.h"

namespace bpp {

bpp_method::bpp_method() {}

bpp_method::bpp_method(const std::string& name) : name(name) {}

/**
 * @brief Add a pointer to a non-primitive object as a parameter to the method
 * 
 * This function ensures that code within the scope of the method is able to access an object
 * 	passed as an argument to the method, by adding the object to the local_objects map.
 */
bool bpp_method::add_object_as_parameter(std::shared_ptr<bpp_object> object) {
	std::string name = object->get_name();
	
	// Verify the object's name isn't already in use by another object
	if (objects.find(name) != objects.end() || local_objects.find(name) != local_objects.end()) {
		return false;
	}

	// Verify the object's name doesn't conflict with a class name
	if (classes.find(name) != classes.end()) {
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
 * This function adds a parameter to the method.
 * If the passed bpp_method_parameter is not a primitive type, it will also add the object to the local_objects map
 *   by calling add_object_as_parameter.
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
		object->set_definition_position(
			parameter->get_initial_definition().file,
			parameter->get_initial_definition().line,
			parameter->get_initial_definition().column
		); // Preserve definition position
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
 * This function destructs all local objects in the method by calling their destructors (if they exist) and then deleting them.
 * 
 * This is called as we exit the method, to ensure that all local objects are cleaned up.
 */
void bpp_method::destruct_local_objects(std::shared_ptr<bpp_program> program) {
	for (auto& o : local_objects) {
		// If it's a pointer, don't delete it
		if (o.second->is_pointer()) {
			continue;
		}

		code_segment delete_code = generate_delete_code(o.second, o.second->get_address(), program);

		*code << delete_code.pre_code << "\n" << std::flush;
	}
	local_objects.clear();
}

bool bpp_method::add_object(std::shared_ptr<bpp_object> object, bool make_local) {
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
		if (make_local) {
			object_code += "local ";
		}
		object_code += object->get_address() + "=\"" + object->get_assignment_value() + "\"\n";
	} else {
		if (object->get_copy_from() != nullptr) {
			object_code += "bpp__" + type + "____copy " + object->get_copy_from()->get_address() + " " + object->get_address() + "\n";
		} else {
			object_code += inline_new(object->get_address(), object->get_class()).pre_code;
			// Call the constructor if it exists
			if (object->get_class()->get_method_UNSAFE("__constructor") != nullptr) {
				object_code += "bpp__" + type + "____constructor " + object->get_address() + "\n";
			}
		}
	}

	*code << object_code << std::flush;
	return true;
}

} // namespace bpp
