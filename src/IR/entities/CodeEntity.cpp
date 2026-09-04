/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "CodeEntity.h"
#include "Object.h"

#include <error/InternalError.h>

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

std::shared_ptr<Object> CodeEntity::getObject(const std::string& name, std::size_t max_visible_index) const {
	auto obj = local_objects.find(name, max_visible_index);
	if (obj) return obj;

	// If not found in local objects, check parent entities
	// This is precisely the procedure for ordinary (non-code) entities, which can't contain their own local objects
	return Entity::getObject(name, max_visible_index);
}

std::vector<std::shared_ptr<Object>> CodeEntity::getAllKnownObjects() const {
	std::vector<std::shared_ptr<Object>> result;

	if (auto parent = getParentEntity().lock()) {
		auto parent_objects = parent->getAllKnownObjects();
		result.insert(result.end(), parent_objects.begin(), parent_objects.end());
	}

	// Add local objects
	const auto& local_objs = local_objects.get_entities();
	result.insert(result.end(), local_objs.begin(), local_objs.end());

	return result;
}

size_t CodeEntity::getNumberOfKnownObjects() const {
	std::size_t count = local_objects.size();

	if (auto parent = getParentEntity().lock()) {
		count += parent->getNumberOfKnownObjects();
	}

	return count;
}

bool CodeEntity::addObject(std::shared_ptr<Object> object) {
	if (getObject(object->getName())) return false; // Object with this name already exists
	if (getClass(object->getName())) return false; // Name conflicts with an existing class

	// Add the object to our local list of owned objects, so that it can be found by name later
	return local_objects.add(object);
}

void CodeEntity::adoptObjectsOf(std::shared_ptr<CodeEntity> other) {
	const auto& objects = other->getLocalObjects().get_entities();
	for (const auto& obj : objects) {
		bpp_assert(obj != nullptr, "Null object in other CodeEntity's local objects");
		bpp_assert(
			this->getObject(obj->getName()) == nullptr,
			"Name conflict when adopting local objects from another CodeEntity: " + obj->getName()
		);
		local_objects.add(obj);
	}
}

bpp::CodeGen::CodeSegment CodeEntity::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "CodeEntity::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code_segment;

	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			code_segment.copy_to_main_code(std::get<RawCode>(child));
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			auto child_entity = std::get<std::shared_ptr<Entity>>(child);
			code_segment.absorb_all_to_main(child_entity->generateCode(state));
		}
	}

	return code_segment;
}

PRETTYPRINT_IMPLEMENTATION(CodeEntity, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');

	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			os << indent;
			prettyprint_raw_code(os, std::get<RawCode>(child));
			os << "\n";
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			std::get<std::shared_ptr<Entity>>(child)->prettyPrint(os, indentation_level);
		}
	}
	return os;
})

} // namespace bpp::IR
