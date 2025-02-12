/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BASH_IF_BRANCH_CPP_
#define SRC_BPP_INCLUDE_BASH_IF_BRANCH_CPP_

#include "bpp.h"

namespace bpp {

bash_if_branch::bash_if_branch() {}

bool bash_if_branch::add_object(std::shared_ptr<bpp_object> object) {
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

	*code << object_code << std::flush;
	return true;
}

void bash_if_branch::set_if_statement(std::shared_ptr<bpp::bash_if> if_statement) {
	this->if_statement = if_statement;
}

std::shared_ptr<bpp::bash_if> bash_if_branch::get_if_statement() const {
	return if_statement;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_IF_BRANCH_CPP_
