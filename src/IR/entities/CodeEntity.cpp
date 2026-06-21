/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "CodeEntity.h"
#include "Object.h"

namespace bpp::IR {

void CodeEntity::add(const RawCode& child) {
	// If the previous child is also raw code, merge this raw code with the previous one.
	if (!children.empty() && std::holds_alternative<RawCode>(children.back())) {
		std::get<RawCode>(children.back()) += child;
	} else {
		children.emplace_back(child);
	}
}

void CodeEntity::add(const std::shared_ptr<Entity>& child) {
	children.emplace_back(child);
}

std::shared_ptr<Object> CodeEntity::get_object(const std::string& name, size_t max_visible_index) {
	auto obj = local_objects.find(name, max_visible_index);
	if (obj) return obj;

	// If not found in local objects, check parent entities
	// This is precisely the procedure for ordinary (non-code) entities, which can't contain their own local objects
	return Entity::get_object(name, max_visible_index);
}

std::vector<std::shared_ptr<Object>> CodeEntity::get_all_known_objects() const {
	std::vector<std::shared_ptr<Object>> result;

	if (auto parent = parent_entity.lock()) {
		auto parent_objects = parent->get_all_known_objects();
		result.insert(result.end(), parent_objects.begin(), parent_objects.end());
	}

	// Add local objects
	const auto& local_objs = local_objects.get_entities();
	result.insert(result.end(), local_objs.begin(), local_objs.end());

	return result;
}

size_t CodeEntity::number_of_known_objects() const {
	size_t count = local_objects.size();

	if (auto parent = parent_entity.lock()) {
		count += parent->number_of_known_objects();
	}

	return count;
}

bool CodeEntity::add_object(std::shared_ptr<Object> object) {
	if (get_object(object->get_name())) return false; // Object with this name already exists
	if (get_class(object->get_name())) return false; // Name conflicts with an existing class

	// Add the object to 2 places:

	// 1. To our local list of owned objects, so that it can be found by name later
	local_objects.add(object);

	// 2. To the entity tree, so that it can be traversed later
	//    (e.g., its position in the entity tree signifies where its instantiation should be placed)
	this->add(object);

	return true;
}

std::ostream& CodeEntity::prettyPrint(std::ostream& os, size_t indentation_level) const {
	std::string indent(indentation_level * 4, ' ');
	os << indent << "(\n";
	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			auto str = std::get<RawCode>(child);
			// Replace all newlines in str with "\n"
			os << indent << "    ";
			for (const char c : str) {
				switch (c) {
					case '\n': os << "\\n"; break;
					case '\t': os << "\\t"; break;
					case '\r': os << "\\r"; break;
					default: os << c; break;
				}
			}
			os << "\n";
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			std::get<std::shared_ptr<Entity>>(child)->prettyPrint(os, indentation_level + 1);
		}
	}
	os << indent << ")\n";
	return os;
}

} // namespace bpp::IR
