/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <sys/stat.h>
#include <unistd.h>
#include "../BashppListener.h"
#include "../../exit_code.h"
#include "../../bpp_include/templates.h"

void BashppListener::enterProgram(std::shared_ptr<AST::Program> node) {
	program->set_output_stream(code_buffer);
	program->set_include_paths(include_paths);
	program->set_target_bash_version(target_bash_version.first, target_bash_version.second);

	if (!included) {
		program->set_main_source_file(source_file);
	} else {
		program->add_source_file(source_file);
	}

	if (included) {
		program = included_from->get_program();
		program->set_output_stream(code_buffer);

		// Inherit all of the parent program's included files
		included_files = included_from->get_included_files();
	} else {
		program->add_code("#!/usr/bin/env bash\n");
		program->add_code(bpp_repeat);
	}

	entity_stack.push(program);
	primitive = program->get_primitive_class();	
}

void BashppListener::exitProgram(std::shared_ptr<AST::Program> node) {
	program->flush_code_buffers();

	entity_stack.pop();
	if (!entity_stack.empty()) {
		throw internal_error("entity_stack is not empty after exiting program");
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
		throw internal_error("code_buffer is not a stringstream");
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
