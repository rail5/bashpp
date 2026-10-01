/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/identifiers.h>

#include <IR/entities/Method.h>
#include <IR/entities/MethodParameter.h>
#include <IR/entities/Class.h>
#include <IR/entities/Object.h>
#include <IR/entities/Program.h>
#include <IR/entities/expressions/bpp/DynamicCast.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(MethodDefinition* node) {
	auto* current_class = dynamic_cast<bpp::IR::Class*>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Method definition outside of class body");

	auto new_method = std::make_unique<bpp::IR::Method>();
	new_method->inherit(current_class);

	// Validate name
	if (!bpp::IR::is_valid_identifier(node->NAME())) {
		std::string msg = "Invalid method name: '" + node->NAME().getValue() + "'";
		if (node->NAME().getValue().contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(node->NAME())) msg += " ('" + node->NAME().getValue() + "' is a keyword)";
		throw bpp::ErrorHandling::SyntaxError(this, node, msg);
	}

	new_method->setName(node->NAME());
	new_method->setIsVirtual(node->VIRTUAL());

	switch (node->ACCESSMODIFIER().getValue()) {
		case AccessModifier::PUBLIC: new_method->setScope(bpp::IR::VisibilityScope::PUBLIC); break;
		case AccessModifier::PRIVATE: new_method->setScope(bpp::IR::VisibilityScope::PRIVATE); break;
		case AccessModifier::PROTECTED: new_method->setScope(bpp::IR::VisibilityScope::PROTECTED); break;
		default: throw bpp::ErrorHandling::InternalError("Unknown access modifier in method definition");
	}

	new_method->setDefinitionPosition({
		get_current_source_file(),
		node->NAME().getLine(),
		node->NAME().getCharPositionInLine()
	});

	// 1. The implicit `this` parameter, which is always the first parameter of a method
	if (!new_method->addParameter(current_class->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to user-defined method: " + node->NAME().getValue());
	}

	// 2. The user-defined parameters
	for (const auto& p : node->PARAMETERS()) {
		const auto& param = p.getValue();
		auto param_name = param.name.getValue();
		const bpp::IR::Class* param_type = nullptr; // Primitive by default

		if (param.type.has_value()) {
			auto type_name = param.type.value().getValue();
			param_type = new_method->getClass(type_name);
			if (!param_type) throw bpp::ErrorHandling::SyntaxError(this, p, "Unknown class: " + type_name);

			if (!param.pointer) throw bpp::ErrorHandling::SyntaxError(this, p, "Methods can only accept pointers as parameters, not objects");

			if (!bpp::IR::is_valid_identifier(param_name)) {
				std::string msg = "Invalid parameter name: '" + param_name + "'";
				if (param_name.contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
				if (bpp::IR::is_protected_keyword(param_name)) msg += " ('" + param_name + "' is a keyword)";
				throw bpp::ErrorHandling::SyntaxError(this, p, msg);
			}
		} else {
			// We don't care whether this identifier shares its name with a keyword,
			// but we do care if it contains double underscores, which are reserved for Bash++'s internal use.
			if (param_name.contains("__")) {
				throw bpp::ErrorHandling::SyntaxError(this, p, "Parameter name cannot contain double underscores: '" + param_name + "'");
			}
		}

		auto parameter_entity = std::make_unique<bpp::IR::MethodParameter>();
		parameter_entity->setType(param_type);
		parameter_entity->setIsPointer(param_type != nullptr);
		parameter_entity->setName(param_name);
		parameter_entity->inherit(new_method.get());

		parameter_entity->setDefinitionPosition({
			get_current_source_file(),
			param.name.getLine(),
			param.name.getCharPositionInLine()
		});

		auto res = new_method->addParameter(std::move(parameter_entity));

		if (!res) {
			std::string error_message = "Parameter name conflicts with existing ";
			switch (res.error()) {
				case bpp::IR::NameConflictError::EXISTING_CLASS: error_message += "class"; break;
				case bpp::IR::NameConflictError::EXISTING_OBJECT: error_message += "object"; break;
				case bpp::IR::NameConflictError::EXISTING_PARAMETER: error_message += "parameter"; break;
				default: error_message += "entity"; break;
			}
			error_message += ": " + param_name;
			throw bpp::ErrorHandling::SyntaxError(this, param.name, error_message);
		}
	}

	// Pre-register the method with the class so that it can be referenced before it is fully defined (e.g., for recursive calls)
	current_class->preregisterMethod(new_method.get());

	entity_stack.push(std::move(new_method));
}

template <>
void Listener::exit(MethodDefinition* node) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting MethodDefinition node");
	auto new_method = entity_stack.pop_as<bpp::IR::Method>();

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting MethodDefinition node");
	auto* current_class = entity_stack.top_as<bpp::IR::Class>();

	current_class->unPreregisterMethod(); // Remove the preregistered method pointer, since the method has now been fully defined

	auto res = current_class->addMethod(std::move(new_method));
	if (!res) {
		std::string error_message = "Method '" + node->NAME().getValue() + "' ";
		if (res.error() == bpp::IR::NameConflictError::EXISTING_DATAMEMBER) {
			error_message += "conflicts with existing data member in class '" + current_class->getName() + "'";
		} else {
			error_message += "already defined in class '" + current_class->getName() + "'";
		}
		throw bpp::ErrorHandling::SyntaxError(this, node->NAME(), error_message);
	}
}

} // namespace bpp::AST
