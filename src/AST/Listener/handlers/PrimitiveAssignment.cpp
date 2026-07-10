/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(PrimitiveAssignment* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering PrimitiveAssignment node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	
	// TODO(@rail5): Can we provide more a more useful error message
	// in the case that this is a direct primitive assignment inside of a class body?
	// (i.e., the user probably meant to declare a data member, and forgot the visibility modifier)
	// In that case, the structure of the AST is: Class -> Block -> BashCommandSequence -> BashPipeline -> BashCommand -> PrimitiveAssignment
	// And the topmost entity on the entity tree would **not** be a Class entity

	auto assignment_entity = std::make_shared<bpp::IR::StringType>();
	assignment_entity->inherit(current_code_entity);

	if (node->isLocal()) assignment_entity->add("local ");

	const std::string& variable_name = node->IDENTIFIER().getValue();
	if (variable_name.contains("__")) {
		show_warning(node->IDENTIFIER(),
			ErrorHandling::WarningType::DubiousVariableName,
			"Variable name '" + variable_name + "' contains double underscores, which are reserved for internal use. Rename to avoid collisions with built-ins or generated symbols.");
	}

	assignment_entity->add(variable_name);

	current_code_entity->add(assignment_entity);

	entity_stack.push(assignment_entity);
}

template <>
void Listener::exit(PrimitiveAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting PrimitiveAssignment node");
	auto assignment_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();
}

} // namespace bpp::AST
