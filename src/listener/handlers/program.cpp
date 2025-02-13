/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_PROGRAM_CPP_
#define SRC_LISTENER_HANDLERS_PROGRAM_CPP_

#include <filesystem>
#include <string.h>
#include <sys/stat.h>
#include "../BashppListener.h"

void BashppListener::enterProgram(BashppParser::ProgramContext *ctx) {
	program->set_output_stream(output_stream);

	if (included) {
		program = included_from->get_program();
		program->set_output_stream(output_stream);

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

	entity_stack.pop();
	if (!entity_stack.empty()) {
		throw internal_error("entity_stack is not empty after exiting program");
	}

	if (included) {
		included_from->set_supershell_counter(supershell_counter);
		included_from->add_to_new_counter(new_counter);
		if (program_has_errors) {
			included_from->set_errors();
		}
		return;
	}

	if (program_has_errors) {
		if (!included) {
			unlink(output_file.c_str());
		}
		return;
	}

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
		bpp_exit_code = WEXITSTATUS(system(command.c_str()));
		unlink(output_file.c_str());
	}
}

#endif // SRC_LISTENER_HANDLERS_PROGRAM_CPP_
