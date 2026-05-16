/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

#include <error/InternalError.h>

namespace bpp {

/**
 * @brief Add an object to this entity's list of objects
 * @param object The object to add
 * @return true if the object was added, false if the object already exists
 */
bool bpp_entity::add_object(std::shared_ptr<bpp_object> object, bool /* make_local */) {
	std::string name = object->get_name();
	if (this->get_object(name) != nullptr) return false; // Object already exists

	local_objects.add(object);
	return true;
}

std::shared_ptr<bpp_class> bpp_entity::get_class() {
	return type.lock();
}

std::string bpp_entity::get_address() const {
	return "";
}

void bpp_entity::set_name(const std::string& name) {
	this->name = name;
}

std::string bpp_entity::get_name() const {
	return this->name;
}

/**
 * @brief Get the class which contains this entity
 * 
 * Useful in many cases, for example in the event that this entity is a method of a particular class
 */
std::weak_ptr<bpp::bpp_class> bpp_entity::get_containing_class() {
	return containing_class;
}

std::weak_ptr<bpp::bpp_program> bpp_entity::get_containing_program() {
	return containing_program;
}

bool bpp_entity::set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) {
	this->containing_class = std::move(containing_class);
	return true;
}

void bpp_entity::set_definition_position(const std::string& file, uint64_t line, uint64_t column) {
	initial_definition = bpp::SymbolPosition(file, line, column);
	references.emplace_front(file, line, column); // Set the definition as the entity's first reference
}

bpp::SymbolPosition bpp_entity::get_initial_definition() const {
	return initial_definition;
}

void bpp_entity::add_reference(const std::string& file, uint64_t line, uint64_t column) {
	references.emplace_back(file, line, column);
	// If this is a derived class's version of an overridden method, add the reference to the overridden method as well
	// This is useful in the language server for "find all references" functionality,
	// And, for example, for "rename symbol" -- renaming the original method should rename all overridden versions as well
	auto _overriden_method = overridden_method.lock();
	if (_overriden_method) {
		_overriden_method->add_reference(file, line, column);
	}
}

std::list<bpp::SymbolPosition> bpp_entity::get_references() const {
	return references;
}

/**
 * @brief Inherit from a parent entity
 *
 * @param parent The parent entity to inherit from
 */
void bpp_entity::inherit(std::shared_ptr<bpp_entity> parent) {
	if (containing_program.expired()) {
		auto parent_program = parent->get_containing_program().lock();
		containing_program = parent_program;
	}
	parent_entity = parent;

	parent_visible_object_count_at_creation = parent->number_of_known_objects();
	program_visible_class_count_at_creation = containing_program.lock()->number_of_known_classes();

	bpp_assert(!containing_program.expired(), std::string("Entity ") + this->get_name() + std::string(" does not have a containing program after inheritance"));
}

void bpp_entity::inherit(std::shared_ptr<bpp_program> program) {
	containing_program = program;
	inherit(std::static_pointer_cast<bpp_entity>(program));
}

void bpp_entity::inherit(std::shared_ptr<bpp_class> parent) {
	inherit(std::static_pointer_cast<bpp_entity>(parent));
}

std::shared_ptr<bpp::bpp_class> bpp_entity::get_class(const std::string& name, size_t /*max_visible_index*/) {
	bpp_assert(!containing_program.expired(), std::string("Entity ") + this->get_name() + std::string(" does not have a containing program"));
	return containing_program.lock()->get_class(name, program_visible_class_count_at_creation);
}

std::shared_ptr<bpp::bpp_object> bpp_entity::get_object(const std::string& name, size_t max_visible_index) {
	auto obj = local_objects.find(name, max_visible_index);
	if (obj) return obj;

	if (auto parent = parent_entity.lock()) {
		return parent->get_object(name, parent_visible_object_count_at_creation);
	}

	return nullptr;
}

const bpp::OwnedEntityList<bpp::bpp_object>& bpp_entity::get_local_objects() const {
	return local_objects;
}

std::vector<std::shared_ptr<bpp_class>> bpp_entity::get_all_known_classes() const {
	auto all_classes = containing_program.lock()->get_all_known_classes();

	if (program_visible_class_count_at_creation < all_classes.size()) {
		return {all_classes.begin(), all_classes.begin() + program_visible_class_count_at_creation};
	}

	return all_classes;
}

std::vector<std::shared_ptr<bpp::bpp_object>> bpp_entity::get_all_known_objects() const {
	std::vector<std::shared_ptr<bpp::bpp_object>> result;

	// Get all from the parent entity
	if (auto parent = parent_entity.lock()) {
		auto parent_objects = parent->get_all_known_objects();
		result.insert(result.end(), parent_objects.begin(), parent_objects.end());
	}

	// Concatenate with this entity's owned objects
	const auto& local_objs = local_objects.get_entities();
	result.insert(result.end(), local_objs.begin(), local_objs.end());

	return result;
}

std::shared_ptr<bpp::bpp_class> bpp_entity::get_parent() const {
	if (!parents.empty()) {
		return parents.back().lock();
	}

	return nullptr;
}

size_t bpp_entity::number_of_known_objects() const {
	size_t count = local_objects.size();
	if (auto p = parent_entity.lock()) {
		count += p->number_of_known_objects();
	}
	return count;
}

size_t bpp_entity::number_of_known_classes() const {
	return containing_program.lock()->number_of_known_classes();
}

} // namespace bpp
