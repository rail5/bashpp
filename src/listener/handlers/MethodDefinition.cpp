/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterMethodDefinition(std::shared_ptr<AST::MethodDefinition> node) {
	skip_syntax_errors
	// Verify we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (current_class == nullptr) {
		syntax_error(node, "Method definition outside of class body");
	}

	std::string method_name = node->NAME().getValue();

	// Add the method to entity stack
	std::shared_ptr<bpp::bpp_method> method = std::make_shared<bpp::bpp_method>(method_name);
	method->inherit(program);
	method->set_containing_class(current_class);

	method->set_definition_position(
		source_file,
		node->NAME().getLine(),
		node->NAME().getCharPositionInLine()
	);

	// Set the method's scope
	switch (node->ACCESSMODIFIER().getValue()) {
		case AST::AccessModifier::PUBLIC:
			method->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
			break;
		case AST::AccessModifier::PROTECTED:
			method->set_scope(bpp::bpp_scope::SCOPE_PROTECTED);
			break;
		case AST::AccessModifier::PRIVATE:
			method->set_scope(bpp::bpp_scope::SCOPE_PRIVATE);
			break;
	}

	// If the method is toPrimitive, verify that the scope is public
	if (method->get_name() == "toPrimitive" && method->get_scope() != bpp::bpp_scope::SCOPE_PUBLIC) {
		syntax_error(node->NAME(), "toPrimitive method must be public");
		return;
	}

	// Verify that the method name does not contain a double underscore
	if (method_name.find("__") != std::string::npos) {
		syntax_error(node->NAME(), "Invalid method name: " + method_name + "\nBash++ identifiers cannot contain double underscores");
	}

	// Virtual?
	if (node->VIRTUAL()) {
		method->set_virtual(true);
	}

	if (!current_class->add_method(method)) {
		syntax_error(node->NAME(), "Method redefinition: " + method->get_name());
	}

	// Check the method's parameters in the parameter list
	for (const auto& p : node->PARAMETERS()) {
		auto param = p.getValue();
		std::string param_name = param.name.getValue();
		std::shared_ptr<bpp::bpp_class> type = program->get_primitive_class();
		if (param.type.has_value()) {
			// Non-primitive type
			std::string type_name = param.type.value();
			type = program->get_class(type_name);
			if (type == nullptr) {
				syntax_error(p, "Unknown class: " + type_name);
			}

			type->add_reference(
				source_file,
				p.getLine(),
				p.getCharPositionInLine()
			);

			if (!param.pointer) {
				syntax_error(p, "Methods can only accept pointers as parameters, not objects");
			}

			// Verify that the parameter name is valid
			if (!bpp::is_valid_identifier(param_name)) {
				// If, specifically, it contains a double underscore, we can provide a more specific error message
				if (param_name.find("__") != std::string::npos) {
					syntax_error(param.name, "Invalid parameter name: " + param_name + "\nBash++ identifiers cannot contain double underscores");
				} else {
					syntax_error(param.name, "Invalid parameter name: " + param_name);
				}
			}

			// Run an implicit dynamic cast in the event that the type is non-primitive
			code_segment dynamic_cast_code = generate_dynamic_cast_code(param_name, type->get_name(), program);
			method->add_code_to_previous_line(dynamic_cast_code.pre_code);
			method->add_code_to_next_line(dynamic_cast_code.post_code);
			method->add_code(param_name + "=" + dynamic_cast_code.code + "\n");
			program->increment_dynamic_cast_counter();
		}

		std::shared_ptr<bpp::bpp_method_parameter> parameter = std::make_shared<bpp::bpp_method_parameter>(param_name);
		parameter->set_type(type);

		parameter->set_definition_position(
			source_file,
			param.name.getLine(),
			param.name.getCharPositionInLine()
		);

		if (!method->add_parameter(parameter)) {
			if (parameter->get_class() != primitive && method->get_object(param_name) != nullptr) {
				syntax_error(param.name, "Parameter name conflicts with existing object: " + param_name);
			}
			
			if (parameter->get_class() != primitive && method->get_class(param_name) != nullptr) {
				syntax_error(param.name, "Parameter name conflicts with existing class: " + param_name);
			}

			// If we reach here, the parameter name is already in use
			syntax_error(param.name, "Duplicate parameter: " + param_name);
		}
	}

	entity_stack.push(method);
	in_method = true;
}

void BashppListener::exitMethodDefinition(std::shared_ptr<AST::MethodDefinition> node) {
	skip_syntax_errors
	// Get the method from the entity stack
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	entity_stack.pop();

	// Call destructors for any objects created in the method before we exit it
	method->destruct_local_objects(program);
	method->flush_code_buffers();
	in_method = false;

	program->mark_entity(
		source_file,
		method->get_initial_definition().line,
		method->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		method
	);
}
