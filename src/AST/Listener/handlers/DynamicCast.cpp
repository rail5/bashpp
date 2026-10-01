/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bpp/DynamicCast.h>
#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(DynamicCast* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering DynamicCast node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto dynamic_cast_entity = std::make_unique<bpp::IR::DynamicCast>();
	dynamic_cast_entity->inherit(current_code_entity);

	entity_stack.push(std::move(dynamic_cast_entity));
	nested_dynamic_cast_depth++;
}

template <>
void Listener::exit(DynamicCast* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::DynamicCast>(), "Topmost entity is not a DynamicCast when exiting DynamicCast node");
	auto dynamic_cast_entity = entity_stack.pop_as<bpp::IR::DynamicCast>();
	nested_dynamic_cast_depth--;

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting DynamicCast node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->adoptObjectsOf(dynamic_cast_entity.get());
	current_code_entity->add(std::move(dynamic_cast_entity));
}

template <>
void Listener::enter(DynamicCastTarget* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::DynamicCast>(), "Topmost entity is not a DynamicCast when entering DynamicCastTarget node");
	auto* dynamic_cast_entity = entity_stack.top_as<bpp::IR::DynamicCast>();

	auto target_type_entity = std::make_unique<bpp::IR::StringType>();
	target_type_entity->inherit(dynamic_cast_entity);
	entity_stack.push(std::move(target_type_entity));
}

template <>
void Listener::exit(DynamicCastTarget* node) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting DynamicCastTarget node");
	auto target_type_entity = entity_stack.pop_as<bpp::IR::StringType>();

	bpp_assert(topmost_entity_is<bpp::IR::DynamicCast>(), "Topmost entity is not a DynamicCast when exiting DynamicCastTarget node");
	auto* dynamic_cast_entity = entity_stack.top_as<bpp::IR::DynamicCast>();

	dynamic_cast_entity->adoptObjectsOf(target_type_entity.get());

	// What kind of input did we receive for the target type?
	if (node->TARGETTYPE().has_value()) {
		// The user gave a class name directly
		const auto& class_name = node->TARGETTYPE().value().getValue();
		dynamic_cast_entity->setTargetType(class_name);

		// Verify the class exists, and possibly issue a warning if not
		auto* target_class = dynamic_cast_entity->getClass(class_name);
		if (!target_class) {
			show_warning(
				node,
				bpp::ErrorHandling::WarningType::CastToUnknownClass,
				"Class not found: '" + class_name + "'" + ". This cast may fail at runtime."
			);
		}
	} else {
		// The user gave an expression which will evaluate to a class name at runtime
		dynamic_cast_entity->setTargetType(std::move(target_type_entity));
	}
}

} // namespace bpp::AST
