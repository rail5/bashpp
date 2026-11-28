/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "CodeEntity.h"

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

void CodeEntity::add(std::unique_ptr<Entity> child) {
	children.emplace_back(std::move(child));
}

Object* CodeEntity::getObject(const std::string& name, std::size_t max_visible_index) const {
	// Search local objects first (if allowed)
	const auto number_of_unowned_objects = Entity::getNumberOfKnownObjects();
	if (max_visible_index > number_of_unowned_objects) {
		const auto local_max_visible_index = max_visible_index - number_of_unowned_objects;
		auto* obj = local_objects.find(name, local_max_visible_index);
		if (obj) return obj;
	}

	// If not found in local objects, check parent entities
	// This is precisely the procedure for ordinary (non-code) entities, which can't contain their own local objects
	return Entity::getObject(name, max_visible_index);
}

std::vector<Object*> CodeEntity::getAllKnownObjects() const {
	auto result = Entity::getAllKnownObjects();

	// Add local objects
	const auto& local_objs = local_objects.view_entities(); // returns a std::span<const std::unique_ptr<Object>>
	result.reserve(result.size() + local_objs.size());
	std::transform(local_objs.begin(), local_objs.end(), std::back_inserter(result), [](const std::unique_ptr<Object>& obj_ptr) { return obj_ptr.get(); });

	return result;
}

size_t CodeEntity::getNumberOfKnownObjects() const {
	return Entity::getNumberOfKnownObjects() + local_objects.size();
}

bool CodeEntity::addObject(std::unique_ptr<Object> object) {
	if (getObject(object->getName())) return false; // Object with this name already exists
	if (getClass(object->getName())) return false; // Name conflicts with an existing class

	// Add the object to our local list of owned objects, so that it can be found by name later
	return local_objects.add(std::move(object));
}

void CodeEntity::adoptObjectsOf(CodeEntity* other) {
	auto objects = other->releaseLocalObjects().release_entities();
	for (auto&& obj : objects) {
		bpp_assert(obj != nullptr, "Object pointer in other CodeEntity's local objects is null");
		bpp_assert(
			this->getObject(obj->getName()) == nullptr,
			"Name conflict when adopting local objects from another CodeEntity: " + obj->getName()
		);
		local_objects.add(std::move(obj));
	}
}

bpp::IR::OwnedEntityList<Object> CodeEntity::releaseLocalObjects() {
	return std::move(local_objects);
}

bool CodeEntity::managesObjectLifetimes() const {
	for (const auto& o : local_objects.view_entities()) {
		if (o->isNonprimitive()) return true;
	}
	return false;
}

bpp::CodeGen::CodeSegment CodeEntity::createScopeFrame(bpp::CodeGen::CodeGenState* state, bool isolate_scope, bool force) const {
	bpp_assert(state != nullptr, "State pointer is null");
	if (!force && !managesObjectLifetimes()) return {}; // Don't bother creating a scope frame if we're not going to use it
	state->scope_frames.insert(this);
	bpp::CodeGen::CodeSegment result;
	if (isolate_scope) {
		result.add_main_code("\nlocal __scopeFrames=(0)\n");
	} else {
		result.add_main_code("\n__scopeFrames+=(0)\n");
	}
	return result;
}

bpp::CodeGen::CodeSegment CodeEntity::destroyScopeFrame(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	if (!state->has_scope_frame(this)) return {};
	bpp::CodeGen::CodeSegment result;

	// Destroy objects in reverse order of creation
	result.add_main_code("\nbpp____destroy_objectStack ${__scopeFrames[-1]}\nunset __scopeFrames[-1]\n");
	return result;
}

bpp::CodeGen::CodeSegment CodeEntity::markLoopFrame(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	if (!state->has_scope_frame(this)) return {};
	bpp::CodeGen::CodeSegment result;
	result.add_main_code("\ndeclare -a __loopFrames\n");
	result.add_main_code("__loopFrames+=(${#__scopeFrames[@]})\n");
	return result;
}

bpp::CodeGen::CodeSegment CodeEntity::unmarkLoopFrame(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	if (!state->has_scope_frame(this)) return {};
	bpp::CodeGen::CodeSegment result;
	result.add_main_code("\nunset __loopFrames[-1]\n");
	return result;
}

bpp::CodeGen::CodeSegment CodeEntity::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment code_segment;

	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			code_segment.copy_to_main_code(std::get<RawCode>(child));
		} else if (std::holds_alternative<std::unique_ptr<Entity>>(child)) {
			auto* child_entity = std::get<std::unique_ptr<Entity>>(child).get();
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
		} else if (std::holds_alternative<std::unique_ptr<Entity>>(child)) {
			std::get<std::unique_ptr<Entity>>(child)->prettyPrint(os, indentation_level);
		}
	}
	return os;
})

} // namespace bpp::IR
