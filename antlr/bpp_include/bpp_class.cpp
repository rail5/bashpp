/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_CLASS_CPP_
#define ANTLR_BPP_INCLUDE_BPP_CLASS_CPP_

#include "bpp.h"

namespace bpp {

bpp_class::bpp_class(std::string name) : name(name) {}

bpp_class::bpp_class(const bpp_class& other, std::string name) : name(name) {
	// Handling inheritance
	// Probably a sloppy way to do this -- we just duplicate the parent class
	// TODO(@rail5): Is there a better way to handle inheritance?
	methods = other.get_methods();
	datamembers = other.get_datamembers();
	constructor = other.get_constructor();
	destructor = other.get_destructor();
}

bool bpp_class::add_method(bpp_method method) {
	std::string signature = method.get_signature();
	for (auto& m : methods) {
		if (m.get_signature() == signature) {
			return false;
		}
	}
	methods.push_back(method);
	return true;
}

bool bpp_class::add_datamember(bpp_datamember datamember) {
	std::string name = datamember.get_name();
	for (auto& d : datamembers) {
		if (d.get_name() == name) {
			return false;
		}
	}
	datamembers.push_back(datamember);
	return true;
}

bool bpp_class::set_constructor(bpp_constructor constructor) {
	if (constructor_set) {
		return false;
	}
	this->constructor = constructor;
	return true;
}

bool bpp_class::set_destructor(bpp_destructor destructor) {
	if (destructor_set) {
		return false;
	}
	this->destructor = destructor;
	return true;
}

std::string bpp_class::get_name() const {
	return name;
}

std::vector<bpp_method> bpp_class::get_methods() const {
	return methods;
}

std::vector<bpp_datamember> bpp_class::get_datamembers() const {
	return datamembers;
}

bpp_constructor bpp_class::get_constructor() const {
	return constructor;
}

bpp_destructor bpp_class::get_destructor() const {
	return destructor;
}

bool bpp_class::has_constructor() const {
	return constructor_set;
}

bool bpp_class::has_destructor() const {
	return destructor_set;
}

} // namespace bpp

#endif // ANTLR_BPP_INCLUDE_BPP_CLASS_CPP
