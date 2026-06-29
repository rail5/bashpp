/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Method.h>
#include <IR/entities/Class.h>
#include <IR/entities/Object.h>
#include <IR/entities/Program.h>
#include <IR/entities/expressions/DynamicCast.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(MethodDefinition* node) {
	auto current_class = std::dynamic_pointer_cast<bpp::IR::Class>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Method definition outside of class body");

	auto method = std::make_shared<bpp::IR::Method>();
	method->set_containing_class(current_class);
	method->inherit(current_class);

	// Validate name
	if (!bpp::IR::is_valid_identifier(node->NAME())) {
		std::string msg = "Invalid method name: '" + node->NAME().getValue() + "'";
		if (node->NAME().getValue().contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(node->NAME())) msg += " ('" + node->NAME().getValue() + "' is a keyword)";
		throw bpp::ErrorHandling::SyntaxError(this, node, msg);
	}

	method->set_name(node->NAME());
	method->set_is_virtual(node->VIRTUAL());
	switch (node->ACCESSMODIFIER().getValue()) {
		case AccessModifier::PUBLIC: method->set_scope(bpp::IR::VisibilityScope::PUBLIC); break;
		case AccessModifier::PRIVATE: method->set_scope(bpp::IR::VisibilityScope::PRIVATE); break;
		case AccessModifier::PROTECTED: method->set_scope(bpp::IR::VisibilityScope::PROTECTED); break;
		default: throw bpp::ErrorHandling::InternalError("Unknown access modifier in method definition");
	}

	if (!current_class->add_method(method)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Method '" + method->get_name() + "' already defined in class '" + current_class->get_name() + "'");
	}

	method->set_definition_position({
		get_current_source_file(),
		node->NAME().getLine(),
		node->NAME().getCharPositionInLine()
	});

	if (auto parent_method = method->get_parent_method()) {
		parent_method->add_reference_position({
			get_current_source_file(),
			node->NAME().getLine(),
			node->NAME().getCharPositionInLine()
		});
	}

	// Set up the method's parameters

	// 1. The implicit `this` parameter, which is always the first parameter of a method
	auto this_ptr = std::make_shared<bpp::IR::ThisPtr>(method);
	this_ptr->inherit(method);
	method->add_parameter(this_ptr);

	// 2. The user-defined parameters
	for (const auto& p : node->PARAMETERS()) {
		const auto& param = p.getValue();
		auto param_name = param.name.getValue();
		std::shared_ptr<bpp::IR::Class> param_type = nullptr; // Primitive by default

		if (param.type.has_value()) {
			auto type_name = param.type.value().getValue();
			param_type = method->get_class(type_name);
			if (!param_type) throw bpp::ErrorHandling::SyntaxError(this, p, "Unknown class: " + type_name);

			if (!param.pointer) throw bpp::ErrorHandling::SyntaxError(this, p, "Methods can only accept pointers as parameters, not objects");

			if (!bpp::IR::is_valid_identifier(param_name)) {
				std::string msg = "Invalid parameter name: '" + param_name + "'";
				if (param_name.contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
				if (bpp::IR::is_protected_keyword(param_name)) msg += " ('" + param_name + "' is a keyword)";
				throw bpp::ErrorHandling::SyntaxError(this, p, msg);
			}

			param_type->add_reference_position({
				get_current_source_file(),
				param.type.value().getLine(),
				param.type.value().getCharPositionInLine()
			});
		} else {
			// We don't care whether this identifier shares its name with a keyword,
			// but we do care if it contains double underscores, which are reserved for Bash++'s internal use.
			if (param_name.contains("__")) {
				throw bpp::ErrorHandling::SyntaxError(this, p, "Parameter name cannot contain double underscores: '" + param_name + "'");
			}
		}

		auto parameter_entity = std::make_shared<bpp::IR::MethodParameter>(method);
		parameter_entity->inherit(method);
		parameter_entity->set_type(param_type);
		parameter_entity->set_is_pointer(param_type != nullptr);
		parameter_entity->set_name(param_name);
		parameter_entity->set_containing_class(current_class);

		parameter_entity->set_definition_position({
			get_current_source_file(),
			param.name.getLine(),
			param.name.getCharPositionInLine()
		});

		if (!method->add_parameter(parameter_entity)) {
			if (parameter_entity->get_type().lock()) {
				if (method->get_object(param_name)) throw bpp::ErrorHandling::SyntaxError(this, param.name, "Parameter name conflicts with existing object: " + param_name);
				if (method->get_class(param_name)) throw bpp::ErrorHandling::SyntaxError(this, param.name, "Parameter name conflicts with existing class: " + param_name);
			}
			// If we reach here, the parameter name is already in use
			throw bpp::ErrorHandling::SyntaxError(this, param.name, "Duplicate parameter name: " + param_name);
		}
	}

	entity_stack.push(method);
	in_method = true;
}

template <>
void Listener::exit(MethodDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting MethodDefinition node");
	entity_stack.pop();
	in_method = false;
}

} // namespace bpp::AST
