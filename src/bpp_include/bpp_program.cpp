/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_PROGRAM_CPP_
#define SRC_BPP_INCLUDE_BPP_PROGRAM_CPP_

#include "bpp.h"
#include "templates.h"

namespace bpp {

bpp_program::bpp_program() {
	primitive_class = std::make_shared<bpp_class>();
	primitive_class->set_name("primitive");
	classes["primitive"] = primitive_class;

	// Pre-allocate space for the unordered_maps:
	// classes, objects
	// This is a performance optimization
	classes.reserve(10);
	objects.reserve(20);
}

bool bpp_program::set_containing_class(std::weak_ptr<bpp_class> containing_class) {
	return false;
}

void bpp_program::set_output_stream(std::shared_ptr<std::ostream> output_stream) {
	code = output_stream;
}

std::shared_ptr<bpp::bpp_class> bpp_program::get_primitive_class() const {
	return primitive_class;
}

bool bpp_program::add_class(std::shared_ptr<bpp_class> class_) {
	std::string name = class_->get_name();
	if (classes.find(name) != classes.end()) {
		return false;
	}
	classes[name] = class_;

	// Add the code for the class
	std::string class_code = "";
	class_code += template_new_function;
	class_code += template_delete_function;
	class_code += template_copy_function;

	if (class_->has_constructor()) {
		class_code += template_constructor;
	}
	if (class_->has_destructor()) {
		class_code += template_destructor;
	}

	/**
	 * Each of these templates has placeholders that need to be replaced, in the format %PLACEHOLDER-NAME%
	 * */

	// Replace all instances of %ASSIGNMENTS% with the assignments for the __new function
	std::string assignments = "";
	for (auto& dm : class_->get_datamembers()) {
		assignments += dm->get_pre_access_code() + "\n";
		if (dm->get_class()->get_name() == "primitive") {
			// Is it an array?
			if (dm->is_array()) {
				assignments += "	eval \"${__objectAddress}__" + dm->get_name() + "=" + dm->get_default_value() + "\"\n";
			} else {
				assignments += "	local __objAssignment=" + dm->get_default_value() + "\n";
				assignments += "	eval \"${__objectAddress}__" + dm->get_name() + "=\\$__objAssignment\"\n";
				assignments += "	unset __objAssignment\n";
			}
		} else if (dm->is_pointer()) {
			std::string default_value = dm->get_default_value();
			std::string default_value_preface = "";
			if (!default_value.empty() && default_value[0] == '$') {
				default_value_preface = "\\";
			}
			assignments += "	eval \"${__objectAddress}__" + dm->get_name() + "=" + default_value_preface + default_value + "\"\n";
		} else {
			// Call 'new' in a supershell and assign its output
			assignments += "	bpp____supershell \"${__objectAddress}__" + dm->get_name() + "\" \"bpp__" + dm->get_class()->get_name() + "____new\"\n";
			increment_supershell_counter();
			// Call the constructor if it exists
			if (dm->get_class()->has_constructor()) {
				assignments += "	bpp__" + dm->get_class()->get_name() + "____constructor \"${__objectAddress}__" + dm->get_name() + "\" 1\n";
			}
		}
		assignments += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%ASSIGNMENTS%", assignments);
	assignments = "";

	// Replace all instances of %DELETIONS% with the deletions for the __delete function
	std::string deletions = "";
	for (auto& dm : class_->get_datamembers()) {
		deletions += dm->get_pre_access_code() + "\n";
		if (dm->get_class()->get_name() == "primitive" || dm->is_pointer()) {
			deletions += "	unset ${__objectAddress}__" + dm->get_name() + "\n";
		} else {
			deletions += "	bpp__" + dm->get_class()->get_name() + "____delete ${__objectAddress}__" + dm->get_name() + " 1\n";
			deletions += "	unset ${__objectAddress}__" + dm->get_name() + "\n";
		}
		deletions += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%DELETIONS%", deletions);
	deletions = "";

	// Replace all instances of %COPIES% with the copies for the __copy function
	std::string copies = "";
	for (auto& dm : class_->get_datamembers()) {
		copies += dm->get_pre_access_code() + "\n";
		if (dm->get_class()->get_name() == "primitive" || dm->is_pointer()) {
			copies += "	local __copyFrom__" + dm->get_name() + "=\"${__copyFromAddress}__" + dm->get_name() + "\"\n";
			copies += "	eval \"${__copyToAddress}__" + dm->get_name() + "=\\${!__copyFrom__" + dm->get_name() + "}\"\n";
		} else {
			copies += "	bpp__" + dm->get_class()->get_name() + "____copy ${__copyFromAddress}__" + dm->get_name() + " ${__copyToAddress}__" + dm->get_name() + " 1 1\n";
		}
		copies += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%COPIES%", copies);
	copies = "";

	// Replace %CONSTRUCTORBODY% with the constructor body
	if (class_->has_constructor()) {
		class_code = replace_all(class_code, "%CONSTRUCTORBODY%", class_->get_constructor()->get_code());
	}

	// Replace %DESTRUCTORBODY% with the destructor body
	if (class_->has_destructor()) {
		class_code = replace_all(class_code, "%DESTRUCTORBODY%", class_->get_destructor()->get_code());
	}

	// Replace all instances of %CLASS% with the class name
	class_code = replace_all(class_code, "%CLASS%", class_->get_name());

	// Add the methods
	for (auto& method : class_->get_methods()) {
		std::string method_code = template_method;
		std::string method_name = method->get_name();
		std::string params = "";
		for (size_t i = 0; i < method->get_parameters().size(); i++) {
			params += method->get_parameters()[i]->get_name() + "=\"$" + std::to_string(i + 1) + "\"";
			if (i < method->get_parameters().size() - 1) {
				params += " ";
			}
		}
		method_code = replace_all(method_code, "%CLASS%", class_->get_name());
		method_code = replace_all(method_code, "%SIGNATURE%", method_name);
		method_code = replace_all(method_code, "%PARAMS%", params);
		method_code = replace_all(method_code, "%METHODBODY%", method->get_code());
		class_code += method_code;
	}

	*code << class_code << std::flush;

	return true;
}

bool bpp_program::add_object(std::shared_ptr<bpp_object> object) {
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
			object_code += "bpp__" + type + "____new " + name + " >/dev/null\n";
			// Call the constructor if it exists
			if (object->get_class()->has_constructor()) {
				object_code += "bpp__" + type + "____constructor " + name + " 0\n";
			}
		}
	}

	*code << object_code << std::flush;
	return true;
}

void bpp_program::set_supershell_counter(uint64_t value) {
	supershell_counter = value;
}

void bpp_program::increment_supershell_counter() {
	supershell_counter++;
	if (supershell_counter == 1) {
		// This is the first object to be created in a supershell
		// We need to add the code to create the supershell
		add_code_to_previous_line(bpp_supershell_function);
	}
}

uint64_t bpp_program::get_supershell_counter() const {
	return supershell_counter;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_PROGRAM_CPP
