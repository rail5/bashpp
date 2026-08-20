/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Class.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/expressions/ValueAssignment.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(DatamemberDeclaration* node) {
	auto current_class = std::dynamic_pointer_cast<bpp::IR::Class>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Data member declaration outside of class body");

	auto dm = std::make_shared<bpp::IR::DataMember>();
	dm->inherit(current_class);

	switch (node->ACCESSMODIFIER().getValue()) {
		case AccessModifier::PUBLIC: dm->set_scope(bpp::IR::VisibilityScope::PUBLIC); break;
		case AccessModifier::PRIVATE: dm->set_scope(bpp::IR::VisibilityScope::PRIVATE); break;
		case AccessModifier::PROTECTED: dm->set_scope(bpp::IR::VisibilityScope::PROTECTED); break;
		default: throw bpp::ErrorHandling::InternalError("Unknown access modifier in data member declaration");
	}

	/*
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

		dm->set_definition_position({
			get_current_source_file(),
			id.value().getLine(),
			id.value().getCharPositionInLine()
		});
	}

	entity_stack.push(dm);
}

template <>
void Listener::exit(DatamemberDeclaration* node) {
	bpp_assert(topmost_entity_is<bpp::IR::DataMember>(), "Topmost entity on stack is not a DataMember when exiting DatamemberDeclaration node");
	auto dm = std::static_pointer_cast<bpp::IR::DataMember>(entity_stack.top());
	entity_stack.pop();

	if (dm->is_pointer() && !dm->has_initial_value()) {
		// Pointers should be auto-initialized to @nullptr (0) if no initial value is provided
		auto value_assignment = std::make_shared<bpp::IR::ValueAssignment>();
		value_assignment->inherit(dm);
		value_assignment->set_lvalue_object(dm);
		value_assignment->add("0");
		dm->set_initial_value(value_assignment);
	}

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting DatamemberDeclaration node");
	auto current_class = std::static_pointer_cast<bpp::IR::Class>(entity_stack.top());
	auto res = current_class->add_datamember(dm);
	if (!res) {
		if (res.error() == bpp::IR::AddError::NAME_CONFLICTS_WITH_EXISTING_DATAMEMBER) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Data member name has already been used: @" + current_class->get_name() + "." + dm->get_name());
		}
		if (res.error() == bpp::IR::AddError::NAME_CONFLICTS_WITH_EXISTING_METHOD) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Data member name conflicts with existing method: " + current_class->get_name() + "." + dm->get_name());
		}
	}
}

} // namespace bpp::AST
