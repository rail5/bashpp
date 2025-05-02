/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CODEGEN_H_
#define SRC_BPP_INCLUDE_BPP_CODEGEN_H_

#include <string>
#include <memory>
#include <antlr4-runtime.h>

#include "../internal_error.cpp"

namespace bpp {
// Minimal forward declarations
class bpp_program;
class bash_while_condition;
class bpp_class;
class bpp_object;

/**
 * @struct code_segment
 * @brief A struct to hold (compiled) code segments
 * 
 * Much of the code generation in the compiler is done using code segments.
 * A code segment consists of three parts:
 * - pre_code: Code that should be executed before the main code
 * - code: The main code
 * - post_code: Code that should be executed after the main code
 * 
 * Generally, the pre_code and post_code are used to set up and clean up the environment
 * in which the main code will run.
 */
struct code_segment {
	std::string pre_code;
	std::string code;
	std::string post_code;
};

code_segment generate_supershell_code(
	const std::string&						code_to_run,
	bool									in_while_condition,
	std::shared_ptr<bash_while_condition>	current_while_condition,
	std::shared_ptr<bpp::bpp_program>		program
	);

code_segment generate_delete_code(
	std::shared_ptr<bpp_object>			object,
	const std::string&					object_ref,
	std::shared_ptr<bpp::bpp_program>	program
	);

code_segment generate_method_call_code(
	const std::string&					reference_code,
	const std::string&					method_name,
	std::shared_ptr<bpp_class>			assumed_class,
	std::shared_ptr<bpp::bpp_program>	program
	);

code_segment generate_dynamic_cast_code(
	const std::string&					reference_code,
	const std::string&					class_name,
	std::shared_ptr<bpp::bpp_program>	program
	);
}

#endif // SRC_BPP_INCLUDE_BPP_CODEGEN_H_
