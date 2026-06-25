/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/BashPipeline.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashPipeline* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when entering BashPipeline node");
	auto current_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	auto pipeline_entity = std::make_shared<bpp::IR::BashPipeline>();
	pipeline_entity->set_containing_class(current_entity->get_containing_class().lock());
	pipeline_entity->inherit(current_entity);
	current_entity->add(pipeline_entity);
	entity_stack.push(pipeline_entity);
}

template <>
void Listener::exit(BashPipeline* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashPipeline>(), "Topmost entity on stack is not a BashPipeline when exiting BashPipeline node");
	auto pipeline_entity = std::static_pointer_cast<bpp::IR::BashPipeline>(entity_stack.top());
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashPipeline node");
	auto current_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_entity->adopt_objects_of(pipeline_entity);
	entity_stack.pop();
}

template <>
void Listener::enter(BashCommandSequence* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when entering BashCommandSequence node");
	std::shared_ptr<bpp::IR::CodeEntity> current_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	auto command_sequence_entity = std::make_shared<bpp::IR::CodeEntity>();
	command_sequence_entity->set_containing_class(current_entity->get_containing_class().lock());
	command_sequence_entity->inherit(current_entity);
	current_entity->add(command_sequence_entity);
	entity_stack.push(command_sequence_entity);
}

template <>
void Listener::exit(BashCommandSequence* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashCommandSequence node");
	auto command_sequence_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashCommandSequence node");
	auto current_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_entity->adopt_objects_of(command_sequence_entity);
	entity_stack.pop();
}

} // namespace bpp::AST
