/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_BASHPPLISTENER_CPP_
#define SRC_LISTENER_BASHPPLISTENER_CPP_

#include "BashppListener.h"

BashppListener::code_segment BashppListener::generate_supershell_code(const std::string& code_to_run_in_supershell) {
	BashppListener::code_segment result;

	uint64_t supershell_counter = program->get_supershell_counter();

	std::string supershell_function_name = "____supershellRunFunc" + std::to_string(supershell_counter);
	std::string supershell_output_variable = "____supershellOutput" + std::to_string(supershell_counter);

	result.pre_code += "function " + supershell_function_name + "() {\n";
	result.pre_code += "	" + code_to_run_in_supershell + "\n";
	result.pre_code += "}\n";

	if (in_while_condition) {
		current_while_condition->add_supershell_function_call("bpp____supershell " + supershell_output_variable + " " + supershell_function_name);
		current_while_condition->increment_supershell_count();
	} else {
		result.pre_code += "bpp____supershell " + supershell_output_variable + " " + supershell_function_name + "\n";
	}
	result.post_code += "unset -f " + supershell_function_name + "\n";
	result.post_code += "unset " + supershell_output_variable + "\n";

	result.code = "${" + supershell_output_variable + "}";

	program->increment_supershell_counter();

	return result;
}

BashppListener::code_segment BashppListener::generate_delete_code(std::shared_ptr<bpp::bpp_object> object, const std::string& object_reference_string, bool force_pointer) {
	// The object_reference_string is how the compiled code should refer to the object
	// Ie, if the object is a pointer, this should be the address of the object
	BashppListener::code_segment result;

	std::string delete_function_name = "bpp__" + object->get_class()->get_name() + "____delete";

	bool is_pointer = object->is_pointer() || force_pointer;

	if (object->get_class()->has_destructor()) {
		result.pre_code += "bpp__" + object->get_class()->get_name() + "____destructor " + object_reference_string + " " + (is_pointer ? "1" : "0") + "\n";
	}

	result.pre_code += delete_function_name + " " + object_reference_string + " " + (is_pointer ? "1" : "0") + "\n";

	return result;
}

BashppListener::code_segment BashppListener::generate_method_call_code(const std::string& reference_code, const std::string& method_name, std::shared_ptr<bpp::bpp_class> assumed_class) {
	BashppListener::code_segment result;

	if (assumed_class == nullptr) {
		throw internal_error("Assumed class is null");
	}

	std::shared_ptr<bpp::bpp_method> assumed_method = assumed_class->get_method_UNSAFE(method_name);
	if (assumed_method == nullptr) {
		throw internal_error("Method " + method_name + " not found in class " + assumed_class->get_name());
	}

	// Is the method virtual?
	if (assumed_method->is_virtual()) {
		// Look up the method in the vTable
		result.pre_code = "bpp____vTable__lookup \"" + reference_code + "\" \"" + method_name + "\" __func" + std::to_string(program->get_function_counter()) + "\n";
		result.post_code = "unset __func" + std::to_string(program->get_function_counter()) + "\n";
		result.code = "${!__func" + std::to_string(program->get_function_counter()) + "} " + reference_code + " 1";
		program->increment_function_counter();
	} else {
		result.code = "bpp__" + assumed_class->get_name() + "__" + method_name + " " + reference_code + " 1";
	}

	return result;
}

void BashppListener::set_source_file(std::string source_file) {
	this->source_file = source_file;
}

void BashppListener::set_include_paths(std::shared_ptr<std::vector<std::string>> include_paths) {
	this->include_paths = include_paths;
}

void BashppListener::set_dynamic_linking(bool dynamic_linking) {
	this->dynamic_linking = dynamic_linking;
}

void BashppListener::set_included(bool included) {
	this->included = included;
}

void BashppListener::set_included_from(BashppListener* included_from) {
	this->included_from = included_from;
	include_stack = included_from->get_include_stack();
	include_stack.push(included_from->source_file);
}

void BashppListener::set_errors() {
	program_has_errors = true;
}

void BashppListener::set_output_stream(std::shared_ptr<std::ostream> output_stream) {
	this->output_stream = output_stream;
}

void BashppListener::set_output_file(std::string output_file) {
	this->output_file = output_file;
}

void BashppListener::set_run_on_exit(bool run_on_exit) {
	this->run_on_exit = run_on_exit;
}

void BashppListener::set_arguments(std::vector<char*> arguments) {
	this->arguments = arguments;
}

std::shared_ptr<bpp::bpp_program> BashppListener::get_program() {
	return program;
}

std::set<std::string> BashppListener::get_included_files() {
	return included_files;
}

std::stack<std::string> BashppListener::get_include_stack() {
	return include_stack;
}

#endif // SRC_LISTENER_BASHPPLISTENER_CPP_
