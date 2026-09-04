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

void Entity::inherit(std::shared_ptr<const Entity> parent) {
	if (containing_program.expired()) containing_program = parent->getContainingProgram();
	if (containing_class.expired()) containing_class = parent->getContainingClass();

	parent_entity = parent;

	bpp_assert(!containing_program.expired(),
		std::string("Entity")
			+ (dynamic_cast<const NamedEntity*>(this)
				? std::string(" '" + dynamic_cast<const NamedEntity*>(this)->getName() + "'")
				: std::string(""))
			+ std::string(" does not have a containing program after inheritance"));

	parent_visible_object_count_at_creation = parent->getNumberOfKnownObjects();
	program_visible_class_count_at_creation = containing_program.lock()->getNumberOfKnownClasses();
}

std::shared_ptr<Class> Entity::getClass(const std::string& name, std::size_t /*max_visible_index*/) const {
	bpp_assert(!containing_program.expired(),
		std::string("Entity")
			+ (dynamic_cast<const NamedEntity*>(this)
				? std::string(" '" + dynamic_cast<const NamedEntity*>(this)->getName() + "'")
				: std::string(""))
			+ std::string(" does not have a containing program"));
	const auto containing_program_ptr = containing_program.lock();
	return containing_program_ptr->getClass(name, program_visible_class_count_at_creation);
}

std::shared_ptr<Object> Entity::getObject(const std::string& name, std::size_t /*max_visible_index*/) const {
	if (auto parent = parent_entity.lock()) {
		return parent->getObject(name, parent_visible_object_count_at_creation);
	}

	return nullptr;
}

std::vector<std::shared_ptr<Class>> Entity::getAllKnownClasses() const {
	auto all_classes = containing_program.lock()->getAllKnownClasses();

	if (program_visible_class_count_at_creation < all_classes.size()) {
		const auto visible_count = static_cast<std::vector<std::shared_ptr<Class>>::difference_type>(program_visible_class_count_at_creation);
		return {
			all_classes.begin(),
			all_classes.begin() + visible_count
		};
	}

	return all_classes;
}

std::vector<std::shared_ptr<Object>> Entity::getAllKnownObjects() const {
	std::vector<std::shared_ptr<Object>> result;

	// Get all from the parent entity
	if (auto parent = parent_entity.lock()) {
		auto parent_objects = parent->getAllKnownObjects();
		result.insert(result.end(), parent_objects.begin(), parent_objects.end());
	}

	return result;
}

size_t Entity::getNumberOfKnownObjects() const {
	std::size_t count = 0;

	if (auto parent = parent_entity.lock()) {
		count += parent->getNumberOfKnownObjects();
	}

	return count;
}

size_t Entity::getNumberOfKnownClasses() const {
	bpp_assert(!containing_program.expired(), std::string("Entity does not have a containing program"));
	return containing_program.lock()->getNumberOfKnownClasses();
}

bool Entity::isReferenced() const {
	// If there's a single surviving entity which still references this entity, then this entity is considered "referenced"
	for (const auto& weak_ref : referencing_entities) {
		if (!weak_ref.expired()) return true;
	}
	return false;
}

} // namespace bpp::IR
