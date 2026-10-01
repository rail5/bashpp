/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Entity.h"
#include <IR/entities/Program.h>
#include <IR/entities/Object.h>
#include <IR/entities/NamedEntity.h>

#include <error/InternalError.h>

namespace bpp::IR {

void Entity::inherit(const Entity* parent) {
	if (!containing_program) containing_program = parent->getContainingProgram();
	if (!containing_class) containing_class = parent->getContainingClass();

	parent_entity = parent;

	bpp_assert(containing_program,
		std::string("Entity")
			+ (dynamic_cast<const NamedEntity*>(this)
				? std::string(" '" + dynamic_cast<const NamedEntity*>(this)->getName() + "'")
				: std::string(""))
			+ std::string(" does not have a containing program after inheritance"));

	parent_visible_object_count_at_creation = parent->getNumberOfKnownObjects();
	program_visible_class_count_at_creation = containing_program->getNumberOfKnownClasses();
}

Class* Entity::getClass(const std::string& name, std::size_t /*max_visible_index*/) const {
	bpp_assert(containing_program,
		std::string("Entity")
			+ (dynamic_cast<const NamedEntity*>(this)
				? std::string(" '" + dynamic_cast<const NamedEntity*>(this)->getName() + "'")
				: std::string(""))
			+ std::string(" does not have a containing program"));
	return containing_program->getClass(name, program_visible_class_count_at_creation);
}

Object* Entity::getObject(const std::string& name, std::size_t /*max_visible_index*/) const {
	if (parent_entity) {
		return parent_entity->getObject(name, parent_visible_object_count_at_creation);
	}

	return nullptr;
}

std::vector<Class*> Entity::getAllKnownClasses() const {
	auto all_classes = containing_program->getAllKnownClasses();

	if (program_visible_class_count_at_creation < all_classes.size()) {
		const auto visible_count = static_cast<std::vector<Class*>::difference_type>(program_visible_class_count_at_creation);
		return {
			all_classes.begin(),
			all_classes.begin() + visible_count
		};
	}

	return all_classes;
}

std::vector<Object*> Entity::getAllKnownObjects() const {
	std::vector<Object*> result;

	// Get all from the parent entity
	if (parent_entity) {
		auto parent_objects = parent_entity->getAllKnownObjects();
		result.insert(result.end(), parent_objects.begin(), parent_objects.end());
	}

	return result;
}

size_t Entity::getNumberOfKnownObjects() const {
	std::size_t count = 0;

	if (parent_entity) {
		count += parent_entity->getNumberOfKnownObjects();
	}

	return count;
}

size_t Entity::getNumberOfKnownClasses() const {
	bpp_assert(containing_program, std::string("Entity does not have a containing program"));
	return containing_program->getNumberOfKnownClasses();
}

} // namespace bpp::IR
