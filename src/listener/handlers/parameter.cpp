/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_PARAMETER_CPP_
#define SRC_LISTENER_HANDLERS_PARAMETER_CPP_

#include "../BashppListener.h"

void BashppListener::enterParameter(BashppParser::ParameterContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	// Verify we're in a method
	std::shared_ptr<bpp::bpp_method> current_method = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	if (current_method == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(0), "Parameter outside of method definition");
	}

	// If there's more than one identifier, it's a non-primitive type
	std::shared_ptr<bpp::bpp_class> type = program->get_primitive_class();
	antlr4::tree::TerminalNode* name;

	if (ctx->IDENTIFIER().size() > 1) {
		std::string type_name = ctx->IDENTIFIER(0)->getText();
		type = program->get_class(type_name);
		if (type == nullptr) {
			throw_syntax_error(ctx->IDENTIFIER(0), "Unknown class: " + type_name);
		}
		name = ctx->IDENTIFIER(1);

		// Run an implicit dynamic cast in the event that the type is non-primitive
		code_segment dynamic_cast_code = generate_dynamic_cast_code(name->getText(), type->get_name());
		current_method->add_code_to_previous_line(dynamic_cast_code.pre_code);
		current_method->add_code_to_next_line(dynamic_cast_code.post_code);
		current_method->add_code(name->getText() + "=" + dynamic_cast_code.code);
		program->increment_dynamic_cast_counter();
	} else {
		name = ctx->IDENTIFIER(0);
	}

	std::shared_ptr<bpp::bpp_method_parameter> parameter = std::make_shared<bpp::bpp_method_parameter>(name->getText());
	parameter->set_type(type);

	if (!current_method->add_parameter(parameter)) {
		throw_syntax_error(name, "Duplicate parameter: " + name->getText());
	}
}

void BashppListener::exitParameter(BashppParser::ParameterContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_PARAMETER_CPP_
