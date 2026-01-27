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
#include <type_traits>

#include "bpp.h"
#include <error/InternalError.h>
#include <AST/ASTNode.h>

namespace bpp {
// Minimal forward declarations
class bpp_program;
class bash_while_or_until_condition;
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
	std::shared_ptr<bpp::bpp_program>		program
	);

code_segment generate_delete_code(
	std::shared_ptr<bpp_object>			object,
	const std::string&					object_ref,
	std::shared_ptr<bpp::bpp_program>	program
	);

code_segment generate_constructor_call_code(
	const std::string&					reference_code,
	std::shared_ptr<bpp_class>			assumed_class
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

code_segment generate_typeof_code(
	const std::string&					reference_code,
	std::shared_ptr<bpp::bpp_program>	program
	);

code_segment inline_new(
	const std::string&					new_address,
	std::shared_ptr<bpp::bpp_class>		new_class
	);

std::string get_encased_ref(const std::string& ref, uint8_t indirection_level);


// Entity reference resolution

struct entity_reference {
	std::shared_ptr<bpp::bpp_entity> entity;
	code_segment reference_code;
	bool created_first_temporary_variable = false;
	bool created_second_temporary_variable = false;
	std::shared_ptr<bpp::bpp_class> class_containing_the_method;
	
	struct reference_error {
		std::string message;
		AST::Token<std::string> token;
	};

	std::optional<reference_error> error;
};

entity_reference resolve_reference_impl(
	const std::string& file,
	std::shared_ptr<bpp::bpp_entity> context,
	std::deque<AST::Token<std::string>>* identifiers,
	std::deque<std::string>* identifier_texts,
	bool declare_local,
	std::shared_ptr<bpp::bpp_program> program
);

inline entity_reference resolve_reference(
	const std::string& file,
	std::shared_ptr<bpp::bpp_entity> context,
	auto identifiers,
	bool declare_local,
	std::shared_ptr<bpp::bpp_program> program
) {
	// identifiers should be either:
	// 1. A pointer to a container of strings (eg, std::deque<std::string>*)
	// 2. A pointer to a container of AST::Token<std::string> (eg, std::deque<AST::Token<std::string>>*)

	using ident_t = std::remove_cvref_t<decltype(identifiers)>;
	using container_t = std::remove_pointer_t<ident_t>;
	using value_t = typename container_t::value_type;

	std::deque<AST::Token<std::string>> node_deque;
	std::deque<std::string> text_deque;

	
	if constexpr (std::is_convertible_v<value_t, AST::Token<std::string>>) {
		// identifiers: deque<TerminalNode*>*
		for (const auto& node : *identifiers) {
			node_deque.push_back(node);
		}
		for (const auto& node : node_deque) {
			text_deque.push_back(node.getValue());
		}
		return resolve_reference_impl(file, context, &node_deque, &text_deque, declare_local, program);
	} else if constexpr (std::is_convertible_v<value_t, std::string>) {
		// identifiers: deque<string>*
		for (const auto& id : *identifiers) {
			text_deque.push_back(id);
		}
		return resolve_reference_impl(file, context, &node_deque, &text_deque, declare_local, program);
	} else {
		static_assert(sizeof(value_t) == 0,
			"resolve_reference: Identifiers must be either strings or TerminalNode pointers.");
	}

}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CODEGEN_H_
