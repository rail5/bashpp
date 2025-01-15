/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_PROGRAM_CPP_
#define ANTLR_BPP_INCLUDE_BPP_PROGRAM_CPP_

#include "bpp.h"
#include "templates.h"

namespace bpp {

bpp_program::bpp_program() {}

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
		if (dm->get_type() == "primitive") {
			assignments += "	local __objAssignment=" + dm->get_default_value() + "\n";
			assignments += "	eval \"${__objectAddress}__" + dm->get_name() + "=\\$__objAssignment\"\n";
			assignments += "	unset __objAssignment\n";
		} else {
			assignments += "	bpp__" + dm->get_type() + "____new \"\" ${__objectAddress}__" + dm->get_name() + "\n";
		}
		assignments += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%ASSIGNMENTS%", assignments);

	// Replace all instances of %DELETIONS% with the deletions for the __delete function
	std::string deletions = "";
	for (auto& dm : class_->get_datamembers()) {
		deletions += dm->get_pre_access_code() + "\n";
		if (dm->get_type() == "primitive") {
			deletions += "	unset ${__objectAddress}__" + dm->get_name() + "\n";
		} else {
			deletions += "	bpp__" + dm->get_type() + "____delete ${__objectAddress}__" + dm->get_name() + " 1\n";
			deletions += "	unset ${__objectAddress}__" + dm->get_name() + "\n";
		}
		deletions += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%DELETIONS%", deletions);

	// Replace all instances of %COPIES% with the copies for the __copy function
	std::string copies = "";
	for (auto& dm : class_->get_datamembers()) {
		copies += dm->get_pre_access_code() + "\n";
		if (dm->get_type() == "primitive") {
			copies += "	local __copyFrom__" + dm->get_name() + "=\"${__copyFromAddress}__" + dm->get_name() + "\"\n";
			copies += "	eval \"${__copyToAddress}__" + dm->get_name() + "=\\${!__copyFrom__" + dm->get_name() + "}\"\n";
		} else {
			copies += "	bpp__" + dm->get_type() + "____copy ${__copyFromAddress}__" + dm->get_name() + " ${__copyToAddress}__" + dm->get_name() + " 1 1\n";
		}
		copies += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%COPIES%", copies);

	// Replace %CONSTRUCTORBODY% with the constructor body
	if (class_->has_constructor()) {
		class_code = replace_all(class_code, "%CONSTRUCTORBODY%", class_->get_constructor()->get_method_body());
	}

	// Replace %DESTRUCTORBODY% with the destructor body
	if (class_->has_destructor()) {
		class_code = replace_all(class_code, "%DESTRUCTORBODY%", class_->get_destructor()->get_method_body());
	}

	// Replace all instances of %CLASS% with the class name
	class_code = replace_all(class_code, "%CLASS%", class_->get_name());

	// Add the methods
	bool found_toPrimitive = false;
	for (auto& method : class_->get_methods()) {
		std::string method_code = template_method;
		std::string signature = method->get_signature();
		if (signature == "toPrimitive") {
			found_toPrimitive = true;
		}
		std::string params = "";
		for (size_t i = 0; i < method->get_parameters().size(); i++) {
			params += method->get_parameters()[i]->get_name() + "=\"$" + std::to_string(i + 3) + "\"";
			if (i < method->get_parameters().size() - 1) {
				params += " ";
			}
		}
		method_code = replace_all(method_code, "%CLASS%", class_->get_name());
		method_code = replace_all(method_code, "%SIGNATURE%", signature);
		method_code = replace_all(method_code, "%PARAMS%", params);
		method_code = replace_all(method_code, "%METHODBODY%", method->get_method_body());
		class_code += method_code;
	}

	// Is there a user-defined toPrimitive method?
	if (!found_toPrimitive) {
		class_code += replace_all(template_toPrimitive, "%CLASS%", class_->get_name());
	}

	code += class_code;

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
		object_code += "bpp__" + type + "____new \"\" " + name + "\n";
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

void bpp_program::add_code(std::string code) {
	this->code += code;
}

std::vector<std::shared_ptr<bpp_class>> bpp_program::get_classes() const {
	std::vector<std::shared_ptr<bpp_class>> result;
	for (auto& c : classes) {
		result.push_back(c.second);
	}
	return result;
}

std::vector<std::shared_ptr<bpp_object>> bpp_program::get_objects() const {
	std::vector<std::shared_ptr<bpp_object>> result;
	for (auto& o : objects) {
		result.push_back(o.second);
	}
	return result;
}

std::string bpp_program::get_code() const {
	return code;
}

std::shared_ptr<bpp::bpp_class> bpp_program::get_class(std::string name) {
	if (classes.find(name) == classes.end()) {
		return nullptr;
	}
	return classes[name];
}

std::shared_ptr<bpp::bpp_object> bpp_program::get_object(std::string name) {
	if (objects.find(name) == objects.end()) {
		return nullptr;
	}
	return objects[name];
}

} // namespace bpp

#endif // ANTLR_BPP_INCLUDE_BPP_PROGRAM_CPP
