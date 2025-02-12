/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_BASHPPLISTENER_CPP_
#define SRC_LISTENER_BASHPPLISTENER_CPP_

#include "BashppListener.h"

BashppListener::code_segment BashppListener::generate_supershell_code(std::string code_to_run_in_supershell) {
	BashppListener::code_segment result;

	std::string supershell_function_name = "____supershellRunFunc" + std::to_string(supershell_counter);
	std::string supershell_output_variable = "____supershellOutput" + std::to_string(supershell_counter);

	result.pre_code += "function " + supershell_function_name + "() {\n";
	result.pre_code += "	" + code_to_run_in_supershell + "\n";
	result.pre_code += "}\n";

	// TODO(@rail5): This HACKY fix does not unset the supershell run-function after while loops end
	// The 'unset -f' command is simply discarded
	// If it wasn't discarded, we would not be able to re-evaluate the supershell on each loop iteration (as the function would be unset)
	// More properly, the function should be unset after the loop ends
	// But it's very hard to determine when the loop ends.
	// We would have to parse the entire loop to determine this, rather than just the first line
	// As it stands, it likely won't cause too much of a problem. But it is unnecessary, persistent memory usage and should be fixed
	if (in_while_statement) {
		current_while_statement->add_supershell_function_call("bpp____supershell " + supershell_output_variable + " " + supershell_function_name);
		current_while_statement->increment_supershell_count();
	} else {
		result.pre_code += "bpp____supershell " + supershell_output_variable + " " + supershell_function_name + "\n";
		result.post_code += "unset -f " + supershell_function_name + "\n";
	}

	result.post_code += "unset " + supershell_output_variable + "\n";

	result.code = "${" + supershell_output_variable + "}";

	supershell_counter++;

	if (supershell_counter == 1) {
		program->add_code_to_previous_line(bpp_supershell_function);
	}

	return result;
}

BashppListener::code_segment BashppListener::generate_new_code(std::shared_ptr<bpp::bpp_class> object_class) {
	BashppListener::code_segment result;

	std::string new_function_name = "bpp__" + object_class->get_name() + "____new";
	std::string new_output_variable = "____newOutput" + std::to_string(new_counter);

	result.pre_code += new_function_name + " \"\" " + new_output_variable + "\n";
	result.code = "${" + new_output_variable + "}";

	if (object_class->has_constructor()) {
		result.post_code += "bpp__" + object_class->get_name() + "____constructor ${" + new_output_variable + "} 1\n";
	}

	result.post_code += "unset " + new_output_variable + "\n";

	new_counter++;

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

void BashppListener::set_source_file(std::string source_file) {
	this->source_file = source_file;
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

void BashppListener::set_supershell_counter(uint64_t value) {
	supershell_counter = value;
}

void BashppListener::add_to_new_counter(uint64_t value) {
	new_counter += value;
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
