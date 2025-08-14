/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CODEGEN_H_
#define SRC_BPP_INCLUDE_BPP_CODEGEN_H_

#include <string>
#include <memory>
#include <optional>
#include <deque>
#include <antlr4-runtime.h>

#include "bpp.h"
#include "../antlr/BashppLexer.h"
#include "../internal_error.h"

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

	/**
	 * @brief Return the full code segment as a single string
	 * 
	 * This function concatenates the pre_code, code, and post_code, and adds separating newlines only if necessary.
	 */
	std::string full_code() const {
		return pre_code + (pre_code.empty() ? "" : "\n") + code + (post_code.empty() ? "" : "\n") + post_code;
	}
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
	bool force_static_reference,
	std::shared_ptr<bpp::bpp_program>	program
	);

code_segment generate_dynamic_cast_code(
	const std::string&					reference_code,
	const std::string&					class_name,
	std::shared_ptr<bpp::bpp_program>	program
	);

code_segment inline_new(
	const std::string&					new_address,
	std::shared_ptr<bpp::bpp_class>		new_class
	);



// Entity reference resolution

struct entity_reference {
	std::shared_ptr<bpp::bpp_entity> entity;
	code_segment reference_code;
	bool created_first_temporary_variable = false;
	bool created_second_temporary_variable = false;
	std::shared_ptr<bpp::bpp_class> class_containing_the_method;
	
	struct reference_error {
		std::string message;
		antlr4::tree::TerminalNode* token = nullptr;
	};

	std::optional<reference_error> error;
};

entity_reference resolve_reference_impl(
	const std::string& file,
	std::shared_ptr<bpp::bpp_entity> context,
	std::deque<antlr4::tree::TerminalNode*>* identifiers,
	std::deque<std::string>* identifier_texts,
	bool declare_local,
	std::shared_ptr<bpp::bpp_program> program
);

inline entity_reference resolve_reference(
	const std::string& file,
	std::shared_ptr<bpp::bpp_entity> context,
	std::deque<std::string>* identifiers,
	bool declare_local,
	std::shared_ptr<bpp::bpp_program> program
) {
	std::deque<antlr4::tree::TerminalNode*> debug_nodes;
	return resolve_reference_impl(file, context, &debug_nodes, identifiers, declare_local, program);
}

inline entity_reference resolve_reference(
	const std::string& file,
	std::shared_ptr<bpp::bpp_entity> context,
	std::deque<antlr4::tree::TerminalNode*>* identifiers,
	bool declare_local,
	std::shared_ptr<bpp::bpp_program> program
) {
	// Extract identifier texts from the nodes
	std::deque<std::string> identifier_texts;
	for (const auto& node : *identifiers) {
		identifier_texts.push_back(node->getText());
	}
	return resolve_reference_impl(file, context, identifiers, &identifier_texts, declare_local, program);
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CODEGEN_H_
