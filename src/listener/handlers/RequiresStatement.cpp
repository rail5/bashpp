/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <listener/BashppListener.h>

/**
 * A 'requires' statement takes the form:
 * @requires COMMAND [ARGUMENTS]
 * For example:
 *
 * @requires curl
 *    This means that the compiled Bash program will check if 'curl' is available on the system before running,
 *    and will exit with an error if 'curl' is not available.
 *
 * @requires curl --version | grep "curl 7.80"
 *    This means that the compiled Bash program will **execute** the command 'curl --version | grep "curl 7.80"'
 *    and check if it exits with a status code of 0 before running,
 *    and will exit with an error if the command exits with a non-zero status code.
 *
 * In short:
 * - If the @requires statement only has a command, then the compiled program will check if the command is available on the system.
 * - If the @requires statement has a command and arguments, then the compiled program will execute the command with the arguments
 *    and check if it exits with a status code of 0.
 *
 * IMPORTANT: All of these checks are performed at the start of the compiled program, before any of the user's code is executed.
 * NOT where the @requires statement is located in the user's code.
 */

void BashppListener::enterRequiresStatement(std::shared_ptr<AST::RequiresStatement> node) {
	// Special case warning:
	//  - If this is a @requires statement with arguments
	//  - And if this statement is not at the top level of the program (i.e. it's inside a function or a class definition)
	// Then we should warn the user that this will be evaluated at the top level of the program,
	// not at the point where it is located in the code.
	//
	// Why warn only this case?
	// 1. @requires statements without arguments have no side effects, so it doesn't matter where they are located in the code.
	// 2. @requires statements with arguments may have side effects (they execute a command),
	//    and the user may expect those side effects to happen at the point where the @requires statement is located in the code,
	//    (e.g. they may incorrectly attempt to use @requires to validate invariants for methods etc)
	//    so it's important in this case to alert the user that this is not how @requires works.
	if (node->hasRequiredArguments() && entity_stack.size() > 1) {
		show_warning(
			node,
			"@requires statements are evaluated at the start of the program, not at the point where they are located in the code"
		);
	}

	// Do not propagate the requirements of dynamically included files to their including files,
	// since a dynamically-included file's requirements will be checked at the point of inclusion
	if (included_status != IncludedStatus::DynamicallyIncluded) {
		program->add_requirement(node->getRequiredCommand().getValue(), node->getRequiredArguments());
	}
}

void BashppListener::exitRequiresStatement(std::shared_ptr<AST::RequiresStatement> node) {}
