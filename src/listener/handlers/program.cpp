/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <filesystem>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../BashppListener.h"
#include "../../exit_code.h"

#include <iostream>
void BashppListener::enterProgram(BashppParser::ProgramContext *ctx) {
	program->set_output_stream(code_buffer);

	if (included) {
		program = included_from->get_program();
		program->set_output_stream(code_buffer);

		// Inherit all of the parent program's included files
		included_files = included_from->get_included_files();
	} else {
		program->add_code("#!/usr/bin/env bash\n");
	}

	entity_stack.push(program);
	primitive = program->get_primitive_class();	
}

void BashppListener::exitProgram(BashppParser::ProgramContext *ctx) {
	program->flush_code_buffers();

	// DEBUG:
	// Dump all symbol information (definition locations, references) to stderr
	auto classes = program->get_classes();
	for (const auto& [name, class_] : classes) {
		if (name == "primitive") {
			continue; // Skip the primitive class
		}
		std::cerr << "Symbol: " << name << std::endl;
		auto initial_definition = class_->get_initial_definition();
		std::cerr << initial_definition.file << ":" << initial_definition.line << ":" << initial_definition.column << std::endl;
		auto references = class_->get_references();
		for (const auto& reference : references) {
			std::cerr <<  reference.file << ":" << reference.line << ":" << reference.column << std::endl;
		}

		std::vector<std::shared_ptr<bpp::bpp_method>> methods = class_->get_methods();
		for (const auto& method : methods) {
			if (method->get_name().find("__") == 0) {
				continue; // Skip system methods
			}
			std::cerr << "Symbol: " << method->get_name() << std::endl;
			auto method_initial_definition = method->get_initial_definition();
			std::cerr << method_initial_definition.file << ":" << method_initial_definition.line << ":" << method_initial_definition.column << std::endl;
			auto method_references = method->get_references();
			for (const auto& method_reference : method_references) {
				std::cerr << method_reference.file << ":" << method_reference.line << ":" << method_reference.column << std::endl;
			}
		}

		std::vector<std::shared_ptr<bpp::bpp_datamember>> datamembers = class_->get_datamembers();
		for (const auto& datamember : datamembers) {
			std::cerr << "Symbol: " << datamember->get_name() << std::endl;
			auto datamember_initial_definition = datamember->get_initial_definition();
			std::cerr << datamember_initial_definition.file << ":" << datamember_initial_definition.line << ":" << datamember_initial_definition.column << std::endl;
			auto datamember_references = datamember->get_references();
			for (const auto& datamember_reference : datamember_references) {
				std::cerr << datamember_reference.file << ":" << datamember_reference.line << ":" << datamember_reference.column << std::endl;
			}
		}
	}

	auto objects = program->get_objects();
	for (const auto& [name, object] : objects) {
		std::cerr << "Symbol: " << name << std::endl;
		auto initial_definition = object->get_initial_definition();
		std::cerr << initial_definition.file << ":" << initial_definition.line << ":" << initial_definition.column << std::endl;
		auto references = object->get_references();
		for (const auto& reference : references) {
			std::cerr << reference.file << ":" << reference.line << ":" << reference.column << std::endl;
		}
	}

	entity_stack.pop();
	if (!entity_stack.empty()) {
		throw internal_error("entity_stack is not empty after exiting program", ctx);
	}

	if (included) {
		if (program_has_errors) {
			included_from->set_errors();
		}
		return;
	}

	if (program_has_errors) {
		bpp_exit_code = EXIT_FAILURE;
		if (!included) {
			unlink(output_file.c_str());
		}
		return;
	}

	// Copy the contents of the code stream to the output stream
	std::shared_ptr<std::ostringstream> cd = std::dynamic_pointer_cast<std::ostringstream>(code_buffer);
	if (cd == nullptr) {
		throw internal_error("code_buffer is not a stringstream", ctx);
	}
	*output_stream << cd->str() << std::flush;
	cd->clear();

	if (!run_on_exit) {
		if (output_file != "") {
			chmod(output_file.c_str(), 0755);
		}
	} else {
		std::string arguments_string = "";
		for (auto argument : arguments) {
			arguments_string += " \"" + replace_all(std::string(argument), "\"", "\\\"") + "\"";
		}

		std::string command = "bash " + output_file + arguments_string;

		// Run the program and get its exit code
		// Below is a small bit of code to compensate for the fact that WEXITSTATUS doesn't work the same on all systems
		// This code will work on all systems
		// The exit code is stored in the lower 8 bits of the return value of the system() function
		bpp_exit_code = (system(command.c_str()) & 0xff00) >> 8;
		unlink(output_file.c_str());
	}
}
