/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_BASHPPLISTENER_CPP_
#define SRC_LISTENER_BASHPPLISTENER_CPP_

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
 * @brief Sets the program_has_errors flag to true.
 * 
 * This function is called when a syntax error is encountered during parsing.
 */
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

void BashppListener::set_suppress_warnings(bool suppress_warnings) {
	this->suppress_warnings = suppress_warnings;
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
