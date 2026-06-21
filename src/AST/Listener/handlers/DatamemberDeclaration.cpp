/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Class.h>
#include <IR/entities/DataMember.h>

namespace bpp::AST {

template <>
void Listener::enter(DatamemberDeclaration* node) {
	auto current_class = std::dynamic_pointer_cast<bpp::IR::Class>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Data member declaration outside of class body");

	auto dm = std::make_shared<bpp::IR::DataMember>();
	dm->set_containing_class(current_class);
	dm->inherit(current_class);

	switch (node->ACCESSMODIFIER().getValue()) {
		case AccessModifier::PUBLIC: dm->set_scope(bpp::IR::VisibilityScope::PUBLIC); break;
		case AccessModifier::PRIVATE: dm->set_scope(bpp::IR::VisibilityScope::PRIVATE); break;
		case AccessModifier::PROTECTED: dm->set_scope(bpp::IR::VisibilityScope::PROTECTED); break;
		default: throw bpp::ErrorHandling::InternalError("Unknown access modifier in data member declaration");
	}

	/**
	 * This will either be:
	 * 	1. A primitive [IDENTIFIER will be set]
	 * 	2. An object [object_instantiation will be set, and we'll handle that in the object_instantiation rule]
	 * 	3. A pointer [pointer_declaration will be set, and we'll handle that in the pointer_declaration rule]
	 */

	const auto& id = node->IDENTIFIER();
	if (id.has_value()) {
		// This is a primitive data member
		dm->set_name(id.value().getValue());

		if (!bpp::IR::is_valid_identifier(dm->get_name())) {
			std::string msg = "Invalid data member name: '" + dm->get_name() + "'";
			if (dm->get_name().contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
			if (bpp::IR::is_protected_keyword(dm->get_name())) msg += " ('" + dm->get_name() + "' is a keyword)";
			throw bpp::ErrorHandling::SyntaxError(this, node, msg);
		}
	}

	dm->set_definition_position({
		source_file,
		id.value().getLine(),
		id.value().getCharPositionInLine()
	});

	entity_stack.push(dm);
}

template <>
void Listener::exit(DatamemberDeclaration* node) {
	bpp_assert(topmost_entity_is<bpp::IR::DataMember>(), "Topmost entity on stack is not a DataMember when exiting DatamemberDeclaration node");
	auto dm = std::static_pointer_cast<bpp::IR::DataMember>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting DatamemberDeclaration node");
	auto current_class = std::static_pointer_cast<bpp::IR::Class>(entity_stack.top());
	if (!current_class->add_datamember(dm)) {
		// Error: naming conflict
		// Let's get specifics on the error: with what does this name conflict?
		auto existing_datamember = current_class->get_datamember_UNSAFE(dm->get_name());
		auto existing_method = current_class->get_method_UNSAFE(dm->get_name());

		if (existing_datamember) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Data member name has already been used: @" + current_class->get_name() + "." + dm->get_name());
		}

		if (existing_method) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Data member name conflicts with existing method: " + current_class->get_name() + "." + dm->get_name());
		}

		// Generic error if we can't determine the cause
		throw bpp::ErrorHandling::SyntaxError(this, node,
			"Data member name '" + dm->get_name() + "' already used in class '" + current_class->get_name() + "'");
	}
}

} // namespace bpp::AST
