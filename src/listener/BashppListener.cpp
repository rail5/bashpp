/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "BashppListener.h"

void BashppListener::set_source_file(std::string source_file) {
	this->source_file = std::move(source_file);
}

void BashppListener::set_include_paths(std::shared_ptr<std::vector<std::string>> include_paths) {
	this->include_paths = std::move(include_paths);
	// Ensure that the standard library directory is always included
	//  and is always the LAST include path
	//  (so that user-provided include paths can override the standard library if needed)
	this->include_paths->emplace_back("/usr/lib/bpp/stdlib/");
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
	include_stack.push_back(included_from->source_file);
}

/**
 * @brief Sets the included_files pointer to the given set of included files.
 */
void BashppListener::set_included_files(std::shared_ptr<std::set<std::string>> included_files) {
	this->included_files = std::move(included_files);
}

void BashppListener::set_code_buffer(std::shared_ptr<std::ostream> code_buffer) {
	this->code_buffer = std::move(code_buffer);
}

void BashppListener::set_output_stream(std::shared_ptr<std::ostream> output_stream) {
	this->output_stream = std::move(output_stream);
}

void BashppListener::set_output_file(std::string output_file) {
	this->output_file = std::move(output_file);
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
	this->arguments = std::move(arguments);
}

void BashppListener::set_lsp_mode(bool lsp_mode) {
	this->lsp_mode = lsp_mode;
}

void BashppListener::set_utf16_mode(bool utf16_mode) {
	this->utf16_mode = utf16_mode;
}

std::shared_ptr<bpp::bpp_program> BashppListener::get_program() const {
	return program;
}

std::shared_ptr<std::set<std::string>> BashppListener::get_included_files() const {
	return included_files;
}

const std::vector<std::string>& BashppListener::get_include_stack() const {
	return include_stack;
}

std::string BashppListener::get_source_file() const {
	return source_file;
}

bool BashppListener::get_lsp_mode() const {
	return lsp_mode;
}

bool BashppListener::get_utf16_mode() const {
	return utf16_mode;
}

int BashppListener::get_exit_code() const {
	return exit_code;
}

bool BashppListener::should_declare_local() const {
	return in_class || in_method || !bash_function_stack.empty();
}

bool BashppListener::should_localize_object_instantiation() const {
	return should_declare_local() && supershell_stack.empty();
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
	replacement_file_contents[file_path] = contents;
}
