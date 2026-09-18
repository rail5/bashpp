/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashTestConditionCommand* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashTestConditionCommand node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	// We create a StringType entity so that the pre&post-code for the contents of the test condition command
	// are place before and after the entire command (respectively), rather than immediately before/after the inner contents
	// e.g.:
	//
	// [[ @obj.member == "some value" ]]
	//
	// Should be turned into:
	//
	// {pre-code to fetch @obj.member}
	// [[ {main-code} == "some value" ]]
	// {post-code}
	//
	// If the entity wasn't a StringType, it would be turned into:
	//
	// [[ {pre-code to fetch @obj.member} {main-code} {post-code} == "some value"  ]]

	auto test_condition_command_entity = std::make_shared<bpp::IR::StringType>();
	test_condition_command_entity->inherit(current_code_entity);
	test_condition_command_entity->add("[[ ");
	entity_stack.push(test_condition_command_entity);
}

template <>
void Listener::exit(BashTestConditionCommand* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashTestConditionCommand node");
	auto test_condition_command_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	test_condition_command_entity->add(" ]]");

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashTestConditionCommand node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(test_condition_command_entity);
}

} // namespace bpp::AST
