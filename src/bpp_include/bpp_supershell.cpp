/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_SUPERSHELL_CPP_
#define SRC_BPP_INCLUDE_BPP_SUPERSHELL_CPP_

#include "bpp.h"

namespace bpp {

bool bpp_supershell::add_object(std::shared_ptr<bpp_object> object) {
	std::string name = object->get_name();
	if (objects.find(name) != objects.end()) {
		return false;
	}

	// Verify that the type of the object is a valid class
	std::string type = object->get_class()->get_name();
	if (classes.find(type) == classes.end()) {
		return false;
	}

	objects[name] = object;

	// Add the code for the object
	std::string object_code = "";

	// Is it a pointer?
	if (object->is_pointer()) {
		object_code += object->get_address() + "=\"" + object->get_assignment_value() + "\"\n";
	} else {
		if (object->get_copy_from() != nullptr) {
			object_code += "bpp__" + type + "____copy " + object->get_copy_from()->get_address() + " " + object->get_address() + " 1 1\n";
		} else {
			object_code += "bpp__" + type + "____new " + name + "\n";
			// Call the constructor if it exists
			if (object->get_class()->has_constructor()) {
				object_code += "bpp__" + type + "____constructor " + name + " 0\n";
			}
		}
	}

	add_code(object_code);
	return true;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_SUPERSHELL_CPP_
