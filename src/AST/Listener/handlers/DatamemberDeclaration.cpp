/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/identifiers.h>

#include <IR/entities/Class.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/expressions/ValueAssignment.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(DatamemberDeclaration* node) {
	auto* current_class = entity_stack.top_as<bpp::IR::Class>();
	if (!current_class) error(node, "Data member declaration outside of class body");

	auto dm = std::make_unique<bpp::IR::DataMember>(node->IDENTIFIER());
	dm->inherit(current_class);

	if (!bpp::IR::is_valid_identifier(node->IDENTIFIER())) {
		std::string msg = "Invalid data member name: '" + node->IDENTIFIER().getValue() + "'";
		if (node->IDENTIFIER().getValue().contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(node->IDENTIFIER())) msg += " ('" + node->IDENTIFIER().getValue() + "' is a keyword)";
		error(node, msg);
	}

	switch (node->ACCESSMODIFIER().getValue()) {
		case AccessModifier::PUBLIC: dm->setScope(bpp::IR::VisibilityScope::PUBLIC); break;
		case AccessModifier::PRIVATE: dm->setScope(bpp::IR::VisibilityScope::PRIVATE); break;
		case AccessModifier::PROTECTED: dm->setScope(bpp::IR::VisibilityScope::PROTECTED); break;
		default: throw bpp::ErrorHandling::InternalError("Unknown access modifier in data member declaration");
	}

	if (node->TYPE().has_value()) {
		const auto& type_name = node->TYPE().value();
		const auto* type = current_class->getClass(type_name);
		if (!type) error(type_name, "Class not found: '" + type_name.getValue() + "'");
		dm->setType(type);
		dm->setIsPointer(node->isPointer());

		if (source_index) type->addReferencePosition(
			{getCurrentSourceFile(),
			type_name.getLine(), type_name.getColumn()}
		);
	}

	if (source_index) dm->setDefinitionPosition(
		{getCurrentSourceFile(),
		node->IDENTIFIER().getLine(), node->IDENTIFIER().getColumn()}
	);

	entity_stack.push(std::move(dm));
}

template <>
void Listener::exit(DatamemberDeclaration* node) {
	bpp_assert(topmost_entity_is<bpp::IR::DataMember>(), "Topmost entity on stack is not a DataMember");
	auto dm = entity_stack.pop_as<bpp::IR::DataMember>();

	if (dm->isPointer() && !dm->hasInitialValue()) {
		// Pointers should be auto-initialized to @nullptr (0) if no initial value is provided
		auto value_assignment = std::make_unique<bpp::IR::ValueAssignment>();
		value_assignment->inherit(dm.get());
		value_assignment->add("0");
		dm->setInitialValue(std::move(value_assignment));
	}

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class");
	auto* current_class = entity_stack.top_as<bpp::IR::Class>();
	auto dm_name = dm->getName();
	auto res = current_class->addDatamember(std::move(dm));
	if (!res) {
		std::string error_message;
		switch (res.error()) {
			case bpp::IR::NameConflictError::EXISTING_DATAMEMBER:
				error_message = "Data member name has already been used: @" + current_class->getName() + "." + dm_name;
				break;
			case bpp::IR::NameConflictError::EXISTING_METHOD:
				error_message = "Data member name conflicts with existing method: " + current_class->getName() + "." + dm_name;
				break;
			default:
				error_message = "Unknown error when adding data member: " + current_class->getName() + "." + dm_name;
				break;
		}
		error(node, error_message);
	}
}

} // namespace bpp::AST
