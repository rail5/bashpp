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
	if (included) {
		program = included_from->get_program();

		// Inherit all of the parent program's included files
		included_files = included_from->get_included_files();
	}

	entity_stack.push(program);
	primitive = program->get_primitive_class();	
}

void BashppListener::exitProgram(BashppParser::ProgramContext *ctx) {
	if (program_has_errors) {
		return;
	}

	program->flush_code_buffers();

	if (!included) {
		if (supershell_counter > 0) {
			program->prepend_code(bpp_supershell_function);
		}

		program->prepend_code("#!/usr/bin/env bash\n");
	}

	entity_stack.pop();
	if (!entity_stack.empty()) {
		throw internal_error("entity_stack is not empty after exiting program");
	}

	if (included) {
		included_from->add_to_supershell_counter(supershell_counter);
		included_from->add_to_new_counter(new_counter);
		return;
	}

	if (!run_on_exit) {
		*output_stream << program->get_code();
		// If the output is a file, mark it as executable
		if (output_file != "") {
			chmod(output_file.c_str(), 0755);
		}
	} else {

		std::string arguments_string = "";
		for (std::string argument : arguments) {
			arguments_string += " \"" + replace_all(argument, "\"", "\\\"") + "\"";
		}

		std::string temp_dir = std::filesystem::temp_directory_path();
		char temp_file[4097];
		strncpy(temp_file, temp_dir.c_str(), 4096);
		strncat(temp_file, "/bashpp_temp_XXXXXX", 4096 - strlen(temp_file));
		int fd = mkstemp(temp_file);
		if (fd == -1) {
			throw std::runtime_error("Failed to create temporary file");
		}
		close(fd);
		std::ofstream temp_stream(temp_file);
		temp_stream << program->get_code();
		temp_stream.close();

		std::string command = "bash " + std::string(temp_file) + arguments_string;

		// Run the program and get its exit code
		bpp_exit_code = WEXITSTATUS(system(command.c_str()));
		unlink(temp_file);
	}
}

#endif // SRC_LISTENER_HANDLERS_PROGRAM_CPP_
