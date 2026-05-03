/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <sys/stat.h>
#include <listener/BashppListener.h>
#include <bpp_include/templates.h>
#include <error/SyntaxError.h>
#include <include/run_bash.h>

void BashppListener::enterProgram(std::shared_ptr<AST::Program> node) {
	program->set_output_stream(code_buffer);
	program->set_include_paths(include_paths);
	program->set_target_bash_version(target_bash_version);

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
	program->set_source_file_ast(source_file, node);

	bpp::ErrorHandling::print_parser_errors(
		parser_errors,
		source_file,
		include_stack,
		program,
		lsp_mode
	);
}

void BashppListener::exitProgram(std::shared_ptr<AST::Program> /*node*/) {
	program->flush_code_buffers();

	entity_stack.pop();
	bpp_assert(entity_stack.empty(), "entity_stack is not empty after exiting program");

	if (included) {
		included_from->set_has_errors(program_has_errors);
		return;
	}

	if (program_has_errors) {
		this->exit_code = EXIT_FAILURE;
		if (!included) {
			unlink(output_file.c_str());
		}
		return;
	}

	// Copy the contents of the code stream to the output stream
	std::shared_ptr<std::ostringstream> cd = std::dynamic_pointer_cast<std::ostringstream>(code_buffer);
	if (cd != nullptr) {
		*output_stream << cd->str() << std::flush;
		cd->clear();
	}
	
	if (!run_on_exit) {
		if (output_file != "") {
			chmod(output_file.c_str(), 0755);
		}
	} else {
		this->exit_code = run_bash(output_file, arguments);
		unlink(output_file.c_str());
	}
}
