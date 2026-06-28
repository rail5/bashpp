/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Entity.h"
#include "Program.h"
#include "Object.h"
#include "NamedEntity.h"

#include <error/InternalError.h>

namespace bpp::IR {

void Entity::inherit(std::shared_ptr<Entity> parent) {
	if (containing_program.expired()) containing_program = parent->get_containing_program();

	parent_entity = parent;

	bpp_assert(!containing_program.expired(),
		std::string("Entity")
			+ (dynamic_cast<const NamedEntity*>(this)
				? std::string(" '" + dynamic_cast<const NamedEntity*>(this)->get_name() + "'")
				: std::string(""))
			+ std::string(" does not have a containing program after inheritance"));

	parent_visible_object_count_at_creation = parent->number_of_known_objects();
	program_visible_class_count_at_creation = containing_program.lock()->number_of_known_classes();
}

std::shared_ptr<Class> Entity::get_class(const std::string& name, std::size_t /*max_visible_index*/) const {
	bpp_assert(!containing_program.expired(), std::string("Entity does not have a containing program"));
	return containing_program.lock()->get_class(name, program_visible_class_count_at_creation);
}

std::shared_ptr<Object> Entity::get_object(const std::string& name, std::size_t /*max_visible_index*/) const {
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
		result.insert(result.end(), parent_objects.begin(), parent_objects.end());
	}

	return result;
}

size_t Entity::number_of_known_objects() const {
	std::size_t count = 0;

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
