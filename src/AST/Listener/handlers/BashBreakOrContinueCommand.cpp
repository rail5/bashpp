/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/BashPipeline.h>
#include <IR/entities/expressions/BashBreakOrContinueCommand.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashBreakOrContinueCommand* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashPipeline>(), "Topmost entity is not a BashPipeline");
	auto current_pipeline = std::static_pointer_cast<bpp::IR::BashPipeline>(entity_stack.top());

	auto break_or_continue_entity = std::make_shared<bpp::IR::BashBreakOrContinueCommand>();
	break_or_continue_entity->inherit(current_pipeline);
	break_or_continue_entity->setIsBreak(node->isBreak());
	break_or_continue_entity->setIsExitPath(node->isExitPath());
	entity_stack.push(break_or_continue_entity);
}

template <>
void Listener::exit(BashBreakOrContinueCommand* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashBreakOrContinueCommand>(), "Topmost entity on stack is not a BashBreakOrContinueCommand");
	auto break_or_continue_entity = std::static_pointer_cast<bpp::IR::BashBreakOrContinueCommand>(entity_stack.top());
	entity_stack.pop();
	bpp_assert(topmost_entity_is<bpp::IR::BashPipeline>(), "Topmost entity on stack is not a BashPipeline");
	auto current_pipeline = std::static_pointer_cast<bpp::IR::BashPipeline>(entity_stack.top());
	current_pipeline->add(break_or_continue_entity);
}

} // namespace bpp::AST
