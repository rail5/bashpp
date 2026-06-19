/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Entity.h"
#include "Program.h"

#include <error/InternalError.h>

namespace bpp::IR {

void Entity::inherit(std::shared_ptr<Entity> parent) {
	if (containing_program.expired()) containing_program = parent->get_containing_program();

	parent_entity = parent;

	bpp_assert(!containing_program.expired(), std::string("Entity does not have a containing program after inheritance"));

	parent_visible_object_count_at_creation = parent->number_of_known_objects();
	program_visible_class_count_at_creation = containing_program.lock()->number_of_known_classes();
}

void Entity::inherit(std::shared_ptr<Program> program) {
	containing_program = program;
	inherit(std::static_pointer_cast<Entity>(program));
}

std::shared_ptr<Class> Entity::get_class(const std::string& name, size_t /*max_visible_index*/) {
	bpp_assert(!containing_program.expired(), std::string("Entity does not have a containing program"));
	return containing_program.lock()->get_class(name, program_visible_class_count_at_creation);
}

std::shared_ptr<Object> Entity::get_object(const std::string& name, size_t max_visible_index) {
	auto obj = local_objects.find(name, max_visible_index);
	if (obj) return obj;

	if (auto parent = parent_entity.lock()) {
		return parent->get_object(name, parent_visible_object_count_at_creation);
	}

	return nullptr;
}

std::vector<std::shared_ptr<Class>> Entity::get_all_known_classes() const {
	auto all_classes = containing_program.lock()->get_all_known_classes();

	if (program_visible_class_count_at_creation < all_classes.size()) {
		return {
			all_classes.begin(),
			all_classes.begin() + program_visible_class_count_at_creation
		};
	}

	return all_classes;
}

std::vector<std::shared_ptr<Object>> Entity::get_all_known_objects() const {
	std::vector<std::shared_ptr<Object>> result;

	// Get all from the parent entity
	if (auto parent = parent_entity.lock()) {
		auto parent_objects = parent->get_all_known_objects();
		result.assign(parent_objects.begin(), parent_objects.end());
	}

	// Concatenate with this entity's owned objects
	const auto& local_objs = local_objects.get_entities();
	result.insert(result.end(), local_objs.begin(), local_objs.end());

	return result;
}

size_t Entity::number_of_known_objects() const {
	size_t count = local_objects.size();

	if (auto parent = parent_entity.lock()) {
		count += parent->number_of_known_objects();
	}

	return count;
}

size_t Entity::number_of_known_classes() const {
	bpp_assert(!containing_program.expired(), std::string("Entity does not have a containing program"));
	return containing_program.lock()->number_of_known_classes();
}

} // namespace bpp::IR
