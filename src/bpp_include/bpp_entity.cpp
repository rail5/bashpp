/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_ENTITY_CPP_
#define SRC_BPP_INCLUDE_BPP_ENTITY_CPP_

#include "bpp.h"

namespace bpp {

bool bpp_entity::add_class(std::shared_ptr<bpp_class> class_) {
	std::string name = class_->get_name();
	if (classes.find(name) != classes.end()) {
		return false;
	}
	classes[name] = class_;
	return true;
}

bool bpp_entity::add_object(std::shared_ptr<bpp_object> object) {
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

std::shared_ptr<bpp_class> bpp_entity::get_class() const {
	return type;
}

std::string bpp_entity::get_address() const {
	return "";
}

std::string bpp_entity::get_name() const {
	return "";
}

std::weak_ptr<bpp::bpp_class> bpp_entity::get_containing_class() const {
	return containing_class;
}

bool bpp_entity::set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) {
	this->containing_class = containing_class;
	return true;
}

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
	if (classes.find(name) == classes.end()) {
		return nullptr;
	}
	return classes[name];
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
