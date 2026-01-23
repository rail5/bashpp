/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "BashppListener.h"

void BashppListener::set_source_file(std::string source_file) {
	this->source_file = source_file;
}

void BashppListener::set_include_paths(std::shared_ptr<std::vector<std::string>> include_paths) {
	this->include_paths = include_paths;
}

void BashppListener::set_included(bool included) {
	this->included = included;
}

/**
 * @brief Sets the included_from pointer to the given listener.
 */
void BashppListener::set_included_from(BashppListener* included_from) {
	this->included_from = included_from;
	include_stack = included_from->get_include_stack();
	include_stack.push(included_from->source_file);
}

/**
 * @brief Sets the included_files pointer to the given set of included files.
 */
void BashppListener::set_included_files(std::shared_ptr<std::set<std::string>> included_files) {
	this->included_files = included_files;
}

/**
 * @brief Sets the program_has_errors flag to true.
 * 
 * This function is called when a syntax error is encountered during parsing.
 */
void BashppListener::set_errors() {
	program_has_errors = true;
}

void BashppListener::set_code_buffer(std::shared_ptr<std::ostream> code_buffer) {
	this->code_buffer = code_buffer;
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

void BashppListener::set_suppress_warnings(bool suppress_warnings) {
	this->suppress_warnings = suppress_warnings;
}

void BashppListener::set_target_bash_version(BashVersion target_bash_version) {
	this->target_bash_version = target_bash_version;
}

void BashppListener::set_arguments(std::vector<char*> arguments) {
	this->arguments = arguments;
}

std::shared_ptr<bpp::bpp_program> BashppListener::get_program() {
	return program;
}

std::shared_ptr<std::set<std::string>> BashppListener::get_included_files() {
	return included_files;
}

std::stack<std::string> BashppListener::get_include_stack() {
	return include_stack;
}

bool BashppListener::should_declare_local() const {
	return in_class || in_method || (!bash_function_stack.empty());
}

std::shared_ptr<bpp::bpp_code_entity> BashppListener::latest_code_entity() {
	// Traverse the entity stack to find the most recent code entity
	std::stack<std::shared_ptr<bpp::bpp_entity>> temp_stack;
	std::shared_ptr<bpp::bpp_code_entity> latest_entity = nullptr;
	while (!entity_stack.empty()) {
		auto entity = entity_stack.top();
		entity_stack.pop();
		temp_stack.push(entity);

		if (std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity) != nullptr) {
			latest_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity);
			break;
		}
	}

	while (!temp_stack.empty()) {
		entity_stack.push(temp_stack.top());
		temp_stack.pop();
	}

	return latest_entity;
}

void BashppListener::set_replacement_file_contents(const std::string& file_path, const std::string& contents) {
	replacement_file_contents = std::make_pair(file_path, contents);
}
