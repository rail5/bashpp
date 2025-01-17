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

	std::cout << program->get_code() << std::endl;
}

#endif // ANTLR_LISTENER_HANDLERS_PROGRAM_CPP_
