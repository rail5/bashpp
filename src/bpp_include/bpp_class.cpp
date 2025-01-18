/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CLASS_CPP_
#define SRC_BPP_INCLUDE_BPP_CLASS_CPP_

#include "bpp.h"

namespace bpp {

bpp_class::bpp_class() {}

bpp_class::bpp_class(std::string name) : name(name) {
	add_default_toPrimitive();
}

bpp_class::bpp_class(const bpp_class& other, std::string name) : name(name) {
	// Handling inheritance
	// Probably a sloppy way to do this -- we just duplicate the parent class
	// TODO(@rail5): Is there a better way to handle inheritance?
	methods = other.get_methods();
	datamembers = other.get_datamembers();
	constructor = other.get_constructor();
	destructor = other.get_destructor();

	add_default_toPrimitive();
}

void bpp_class::set_name(std::string name) {
	this->name = name;

	add_default_toPrimitive();
}

bool bpp_class::add_method(std::shared_ptr<bpp_method> method) {
	std::string name = method->get_name();

	if (name == "toPrimitive" && !has_custom_toPrimitive) {
		remove_default_toPrimitive();
		has_custom_toPrimitive = true;
	}

	for (auto& m : methods) {
		if (m->get_name() == name) {
			return false;
		}
	}
	methods.push_back(method);
	return true;
}

bool bpp_class::add_datamember(std::shared_ptr<bpp_datamember> datamember) {
	std::string name = datamember->get_name();
	for (auto& d : datamembers) {
		if (d->get_name() == name) {
			return false;
		}
	}
	datamembers.push_back(datamember);
	return true;
}

bool bpp_class::set_constructor(std::shared_ptr<bpp_constructor> constructor) {
	if (constructor_set) {
		return false;
	}
	this->constructor = constructor;
	return true;
}

bool bpp_class::set_destructor(std::shared_ptr<bpp_destructor> destructor) {
	if (destructor_set) {
		return false;
	}
	this->destructor = destructor;
	return true;
}

std::string bpp_class::get_name() const {
	return name;
}

std::vector<std::shared_ptr<bpp_method>> bpp_class::get_methods() const {
	return methods;
}

std::vector<std::shared_ptr<bpp_datamember>> bpp_class::get_datamembers() const {
	return datamembers;
}

std::shared_ptr<bpp_constructor> bpp_class::get_constructor() const {
	return constructor;
}

std::shared_ptr<bpp_destructor> bpp_class::get_destructor() const {
	return destructor;
}

bool bpp_class::has_constructor() const {
	return constructor_set;
}

bool bpp_class::has_destructor() const {
	return destructor_set;
}

std::shared_ptr<bpp::bpp_method> bpp_class::get_method(std::string name) {
	for (auto& m : methods) {
		if (m->get_name() == name) {
			return m;
		}
	}
	return nullptr;
}

std::shared_ptr<bpp::bpp_datamember> bpp_class::get_datamember(std::string name) {
	for (auto& d : datamembers) {
		if (d->get_name() == name) {
			return d;
		}
	}
	return nullptr;
}

void bpp_class::inherit(std::shared_ptr<bpp_class> parent) {
	// Inherit methods
	for (auto& m : parent->get_methods()) {
		methods.push_back(m);
	}

	// Inherit datamembers
	for (auto& d : parent->get_datamembers()) {
		datamembers.push_back(d);
	}

	// Inherit constructor
	if (parent->has_constructor()) {
		constructor = parent->get_constructor();
		constructor_set = true;
	}

	// Inherit destructor
	if (parent->has_destructor()) {
		destructor = parent->get_destructor();
		destructor_set = true;
	}
}

void bpp_class::destroy() {
	for (auto& m : methods) {
		m->destroy();
	}
	for (auto& d : datamembers) {
		d->destroy();
	}
	constructor->destroy();
	destructor->destroy();

	name.clear();
	methods.clear();
	datamembers.clear();
	constructor_set = false;
	destructor_set = false;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CLASS_CPP
