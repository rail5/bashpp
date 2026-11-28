/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Entity.h"
#include <IR/entities/Program.h>
#include <IR/entities/Object.h>
#include <IR/entities/components/Named.h>

#include <error/InternalError.h>

namespace bpp::IR {

std::string_view Entity::_getNameIfExists() const {
	if (const auto* named_entity = dynamic_cast<const Components::Named*>(this)) {
		return named_entity->getName();
	}

	return {};
}

void Entity::inherit(const Entity* parent) {
	if (!containing_program) containing_program = parent->getContainingProgram();
	if (!containing_class) containing_class = parent->getContainingClass();

	parent_entity = parent;

	bpp_assert(containing_program,
		std::string("Entity")
			+ std::string(_getNameIfExists())
			+ std::string(" does not have a containing program after inheritance"));

	parent_visible_object_count_at_creation = parent->getNumberOfKnownObjects();
	program_visible_class_count_at_creation = containing_program->getNumberOfKnownClasses();
}

Class* Entity::getClass(const std::string& name, std::size_t max_visible_index) const {
	bpp_assert(containing_program,
		std::string("Entity")
			+ std::string(_getNameIfExists())
			+ std::string(" does not have a containing program"));
	const auto limit = std::min(max_visible_index, program_visible_class_count_at_creation);
	return containing_program->getClass(name, limit);
}

Object* Entity::getObject(const std::string& name, std::size_t max_visible_index) const {
	const auto limit = std::min(max_visible_index, parent_visible_object_count_at_creation);
	if (parent_entity) return parent_entity->getObject(name, limit);
	return nullptr;
}

std::vector<Class*> Entity::getAllKnownClasses() const {
	bpp_assert(containing_program, "Entity" + std::string(_getNameIfExists()) + " does not have a containing program");
	auto all_classes = containing_program->getAllKnownClasses();
	bpp_assert(program_visible_class_count_at_creation <= all_classes.size(), "Entity" + std::string(_getNameIfExists()) + " has a program_visible_class_count_at_creation that is larger than the number of classes in the containing program");
	return {
		all_classes.begin(),
		all_classes.begin() + static_cast<std::vector<Class*>::difference_type>(program_visible_class_count_at_creation)
	};
}

std::vector<Object*> Entity::getAllKnownObjects() const {
	if (parent_entity) {
		auto parent_objects = parent_entity->getAllKnownObjects();
		bpp_assert(parent_visible_object_count_at_creation <= parent_objects.size(), "Entity" + std::string(_getNameIfExists()) + " has a parent_visible_object_count_at_creation that is larger than the number of objects in the parent entity");
		return {
			parent_objects.begin(),
			parent_objects.begin() + static_cast<std::vector<Object*>::difference_type>(parent_visible_object_count_at_creation)
		};
	}
	return {};
}

std::size_t Entity::getNumberOfKnownObjects() const {
	return parent_visible_object_count_at_creation;
}

std::size_t Entity::getNumberOfKnownClasses() const {
	return program_visible_class_count_at_creation;
}

} // namespace bpp::IR
