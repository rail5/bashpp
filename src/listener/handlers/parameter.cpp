/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterParameter(BashppParser::ParameterContext *ctx) {
	skip_syntax_errors
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

		if (ctx->ASTERISK() == nullptr) {
			throw_syntax_error(ctx->IDENTIFIER(0), "Methods can only accept pointers as parameters, not objects");
		}

		// Verify the parameter name is not a protected keyword
		if (is_protected_keyword(name->getText())) {
			throw_syntax_error(name, "Invalid parameter name: " + name->getText());
		}
		// Verify that the parameter name does not contain a double underscore
		if (name->getText().find("__") != std::string::npos) {
			throw_syntax_error(name, "Invalid parameter name: " + name->getText() + "\nBash++ identifiers cannot contain double underscores");
		}

		// Run an implicit dynamic cast in the event that the type is non-primitive
		code_segment dynamic_cast_code = generate_dynamic_cast_code(name->getText(), type->get_name(), program);
		current_method->add_code_to_previous_line(dynamic_cast_code.pre_code);
		current_method->add_code_to_next_line(dynamic_cast_code.post_code);
		current_method->add_code(name->getText() + "=" + dynamic_cast_code.code);
		program->increment_dynamic_cast_counter();
	} else {
		name = ctx->IDENTIFIER(0);
	}

	std::shared_ptr<bpp::bpp_method_parameter> parameter = std::make_shared<bpp::bpp_method_parameter>(name->getText());
	parameter->set_type(type);

	parameter->set_definition_position(
		source_file,
		name->getSymbol()->getLine() - 1,
		name->getSymbol()->getCharPositionInLine()
	);

	if (!current_method->add_parameter(parameter)) {
		if (parameter->get_class() != primitive && current_method->get_object(name->getText()) != nullptr) {
			throw_syntax_error(name, "Parameter name conflicts with existing object: " + name->getText());
		}
		
		if (parameter->get_class() != primitive && current_method->get_class(name->getText()) != nullptr) {
			throw_syntax_error(name, "Parameter name conflicts with existing class: " + name->getText());
		}

		// If we reach here, the parameter name is already in use
		throw_syntax_error(name, "Duplicate parameter: " + name->getText());
	}
}

void BashppListener::exitParameter(BashppParser::ParameterContext *ctx) {
	skip_syntax_errors
}
