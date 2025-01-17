/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_PROGRAM_CPP_
#define ANTLR_LISTENER_HANDLERS_PROGRAM_CPP_

#include "../BashppListener.h"

void BashppListener::enterProgram(BashppParser::ProgramContext *ctx) {
	program->add_code("#!/usr/bin/env bash\n");
	program->add_code(bpp_supershell_function);

	entity_stack.push(program);

	primitive = program->get_primitive_class();	
}

void BashppListener::exitProgram(BashppParser::ProgramContext *ctx) {
	if (program_has_errors) {
		return;
	}

	program->flush_code_buffers();

	entity_stack.pop();
	if (!entity_stack.empty()) {
		throw internal_error("entity_stack is not empty after exiting program");
	}

	//std::cout << program->get_code() << std::endl;
	if (!run_on_exit) {
		*output_stream << program->get_code();
	} else {
		char temp_file[] = "/tmp/bashpp_temp_XXXXXX";
		int fd = mkstemp(temp_file);
		if (fd == -1) {
			throw std::runtime_error("Failed to create temporary file");
		}
		close(fd);
		std::ofstream temp_stream(temp_file);
		temp_stream << program->get_code();
		temp_stream.close();
		system(("bash " + std::string(temp_file)).c_str());
		system(("rm " + std::string(temp_file)).c_str());
	}
}

#endif // ANTLR_LISTENER_HANDLERS_PROGRAM_CPP_
