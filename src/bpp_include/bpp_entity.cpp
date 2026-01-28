/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"

namespace bpp {

/**
 * @brief Add a class to this entity's list of classes
 * @param class_ The class to add
 * @return true if the class was added, false if the class already exists
 */
bool bpp_entity::add_class(std::shared_ptr<bpp_class> class_) {
	std::string name = class_->get_name();
	if (classes.find(name) != classes.end()) {
		return false;
	}
	classes[name] = class_;
	return true;
}

/**
 * @brief Add an object to this entity's list of objects
 * @param object The object to add
 * @return true if the object was added, false if the object already exists
 */
bool bpp_entity::add_object(std::shared_ptr<bpp_object> object, bool make_local) {
	std::string name = object->get_name();
	if (objects.find(name) != objects.end()) {
		return false;
	}

	// Verify that the type of the object is a valid class
	std::string type = object->get_class()->get_name();
	if (classes.find(type) == classes.end()) {
		return false;
	}

	objects[name] = object;
	return true;
}

std::shared_ptr<bpp_class> bpp_entity::get_class() {
	return type;
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

bool bpp_entity::set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) {
	this->containing_class = containing_class;
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
 * This function copies all classes and objects from the parent entity into this entity.
 * 
 * @param parent The parent entity to inherit from
 */
void bpp_entity::inherit(std::shared_ptr<bpp_entity> parent) {
	for (auto& c : parent->get_classes()) {
		classes[c.first] = c.second;
	}
	for (auto& o : parent->get_objects()) {
		objects[o.first] = o.second;
	}
}

void bpp_entity::inherit(std::shared_ptr<bpp_class> parent) {
	inherit(std::dynamic_pointer_cast<bpp_entity>(parent));
}

std::unordered_map<std::string, std::shared_ptr<bpp_class>> bpp_entity::get_classes() const {
	std::unordered_map<std::string, std::shared_ptr<bpp_class>> classes;
	classes.reserve(this->classes.size());
	for (auto& c : this->classes) {
		classes[c.first] = c.second.lock();
	}
	return classes;
}

std::unordered_map<std::string, std::shared_ptr<bpp_object>> bpp_entity::get_objects() const {
	std::unordered_map<std::string, std::shared_ptr<bpp_object>> all_objects;
	all_objects.reserve(objects.size() + local_objects.size());
	for (auto& o : objects) {
		all_objects[o.first] = o.second.lock();
	}
	for (auto& o : local_objects) {
		all_objects[o.first] = o.second;
	}
	return all_objects;
}

std::shared_ptr<bpp::bpp_class> bpp_entity::get_class(const std::string& name) {
	auto it = classes.find(name);
	if (it != classes.end()) {
		return it->second.lock();
	}

	return nullptr;
}

std::shared_ptr<bpp::bpp_object> bpp_entity::get_object(const std::string& name) {
	if (local_objects.find(name) != local_objects.end()) {
		return local_objects[name];
	}

	if (objects.find(name) != objects.end()) {
		return objects[name].lock();
	}

	return nullptr;
}

std::shared_ptr<bpp::bpp_class> bpp_entity::get_parent() const {
	if (!parents.empty()) {
		return parents.back().lock();
	}

	return nullptr;
}

} // namespace bpp
