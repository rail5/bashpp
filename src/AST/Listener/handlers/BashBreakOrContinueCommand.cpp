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
	auto* current_pipeline = entity_stack.top_as<bpp::IR::BashPipeline>();

	auto break_or_continue_entity = std::make_unique<bpp::IR::BashBreakOrContinueCommand>();
	break_or_continue_entity->inherit(current_pipeline);
	break_or_continue_entity->setIsBreak(node->isBreak());
	break_or_continue_entity->setIsExitPath(node->isExitPath());
	entity_stack.push(std::move(break_or_continue_entity));
}

template <>
void Listener::exit(BashBreakOrContinueCommand* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashBreakOrContinueCommand>(), "Topmost entity on stack is not a BashBreakOrContinueCommand");
	auto break_or_continue_entity = entity_stack.pop_as<bpp::IR::BashBreakOrContinueCommand>();

	bpp_assert(topmost_entity_is<bpp::IR::BashPipeline>(), "Topmost entity on stack is not a BashPipeline");
	auto* current_pipeline = entity_stack.top_as<bpp::IR::BashPipeline>();
	current_pipeline->add(std::move(break_or_continue_entity));
}

} // namespace bpp::AST
