/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/DynamicCast.h>
#include <IR/entities/Class.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(DynamicCast* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering DynamicCast node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto dynamic_cast_entity = std::make_shared<bpp::IR::DynamicCast>();
	dynamic_cast_entity->set_containing_class(current_code_entity->get_containing_class().lock());
	dynamic_cast_entity->inherit(current_code_entity);

	entity_stack.push(dynamic_cast_entity);
	dynamic_cast_stack.push({});
}

template <>
void Listener::exit(DynamicCast* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::DynamicCast>(), "Topmost entity is not a DynamicCast when exiting DynamicCast node");
	auto dynamic_cast_entity = std::static_pointer_cast<bpp::IR::DynamicCast>(entity_stack.top());
	entity_stack.pop();
	dynamic_cast_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting DynamicCast node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(dynamic_cast_entity);
	current_code_entity->adopt_objects_of(dynamic_cast_entity);
}

template <>
void Listener::enter(DynamicCastTarget* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::DynamicCast>(), "Topmost entity is not a DynamicCast when entering DynamicCastTarget node");
	auto dynamic_cast_entity = std::static_pointer_cast<bpp::IR::DynamicCast>(entity_stack.top());

	auto target_type_entity = std::make_shared<bpp::IR::StringType>();
	target_type_entity->set_containing_class(dynamic_cast_entity->get_containing_class().lock());
	target_type_entity->inherit(dynamic_cast_entity);
	entity_stack.push(target_type_entity);
}

template <>
void Listener::exit(DynamicCastTarget* node) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting DynamicCastTarget node");
	auto target_type_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::DynamicCast>(), "Topmost entity is not a DynamicCast when exiting DynamicCastTarget node");
	auto dynamic_cast_entity = std::static_pointer_cast<bpp::IR::DynamicCast>(entity_stack.top());

	// What kind of input did we receive for the target type?
	if (node->TARGETTYPE().has_value()) {
		// The user gave a class name directly
		const auto& class_name = node->TARGETTYPE().value().getValue();
		dynamic_cast_entity->set_target_type(class_name);

		// Verify the class exists, and possibly issue a warning if not
		auto target_class = dynamic_cast_entity->get_class(class_name);
		if (!target_class) {
			show_warning(
				node,
				bpp::ErrorHandling::WarningType::CastToUnknownClass,
				"Class not found: '" + class_name + "'" + ". This cast may fail at runtime."
			);
		} else {
			target_class->add_reference_position({
				get_current_source_file(),
				node->TARGETTYPE().value().getLine(),
				node->TARGETTYPE().value().getCharPositionInLine()
			});
		}
	} else {
		// The user gave an expression which will evaluate to a class name at runtime
		dynamic_cast_entity->set_target_type(target_type_entity);
	}

	dynamic_cast_entity->adopt_objects_of(target_type_entity);
}

} // namespace bpp::AST
