/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_ENTITY_CPP_
#define SRC_BPP_INCLUDE_BPP_ENTITY_CPP_

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

std::string bpp_entity::get_name() const {
	return "";
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

/**
 * @brief Inherit from a parent entity
 * 
 * This function copies all classes and objects from the parent entity into this entity.
 * 
 * @param parent The parent entity to inherit from
 */
void bpp_entity::inherit(std::shared_ptr<bpp_entity> parent) {
	for (auto& p : parent->parents) {
		parents.push_back(p);
	}
	parents.push_back(parent->get_class());

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
	return classes;
}

std::unordered_map<std::string, std::shared_ptr<bpp_object>> bpp_entity::get_objects() const {
	std::unordered_map<std::string, std::shared_ptr<bpp_object>> all_objects = objects;
	for (auto& o : local_objects) {
		all_objects[o.first] = o.second;
	}
	return all_objects;
}

std::shared_ptr<bpp::bpp_class> bpp_entity::get_class(const std::string& name) {
	auto it = classes.find(name);
	if (it != classes.end()) {
		return it->second;
	}

	return nullptr;
}

std::shared_ptr<bpp::bpp_object> bpp_entity::get_object(const std::string& name) {
	if (local_objects.find(name) != local_objects.end()) {
		return local_objects[name];
	}

	if (objects.find(name) != objects.end()) {
		return objects[name];
	}

	return nullptr;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_ENTITY_CPP_
