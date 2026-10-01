/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/expressions/bash/BashPipeline.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashCommand* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashPipeline>(), "Topost entity is not a BashPipeline");
	auto* pipeline_entity = entity_stack.top_as<bpp::IR::BashPipeline>();
	pipeline_entity->setExitPointType(node->getExitPointType());
}

template <>
void Listener::exit(BashCommand* /*node*/) {}

template <>
void Listener::enter(BashPipeline* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when entering BashPipeline node");
	auto* current_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	auto pipeline_entity = std::make_unique<bpp::IR::BashPipeline>();
	pipeline_entity->inherit(current_entity);
	entity_stack.push(std::move(pipeline_entity));
}

template <>
void Listener::exit(BashPipeline* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashPipeline>(), "Topmost entity on stack is not a BashPipeline when exiting BashPipeline node");
	auto pipeline_entity = entity_stack.pop_as<bpp::IR::BashPipeline>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashPipeline node");
	auto* current_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_entity->adoptObjectsOf(pipeline_entity.get());
	current_entity->add(std::move(pipeline_entity));
}

template <>
void Listener::enter(BashCommandSequence* node) {
	auto* current_code_entity = dynamic_cast<bpp::IR::CodeEntity*>(entity_stack.top());
	if (!current_code_entity) throw bpp::ErrorHandling::SyntaxError(this, node, "Command sequence outside of a code entity");

	auto command_sequence_entity = std::make_unique<bpp::IR::CodeEntity>();
	command_sequence_entity->inherit(current_code_entity);
	entity_stack.push(std::move(command_sequence_entity));
}

template <>
void Listener::exit(BashCommandSequence* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashCommandSequence node");
	auto command_sequence_entity = entity_stack.pop_as<bpp::IR::CodeEntity>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashCommandSequence node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->adoptObjectsOf(command_sequence_entity.get());
	current_code_entity->add(std::move(command_sequence_entity));
}

} // namespace bpp::AST
