/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_CLASS_CPP_
#define SRC_BPP_INCLUDE_BPP_CLASS_CPP_

#include "bpp.h"

namespace bpp {

bpp_class::bpp_class() {}

bpp_class::bpp_class(const std::string& name) : name(name) {
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

std::weak_ptr<bpp::bpp_class> bpp_class::get_containing_class() const {
	return std::const_pointer_cast<bpp::bpp_class>(this->shared_from_this());
}

std::shared_ptr<bpp_class> bpp_class::get_class() const {
	return std::const_pointer_cast<bpp::bpp_class>(this->shared_from_this());
}

bool bpp_class::set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) {
	return false;
}

void bpp_class::set_name(const std::string& name) {
	this->name = name;

	add_default_toPrimitive();
}

bool bpp_class::add_method(std::shared_ptr<bpp_method> method) {
	std::string name = method->get_name();

	if (name == "toPrimitive" && !has_custom_toPrimitive) {
		// toPrimitive must ALWAYS be public
		if (method->get_scope() != bpp_scope::SCOPE_PUBLIC) {
			return false;
		}
		remove_default_toPrimitive();
		has_custom_toPrimitive = true;
	}

	for (auto it = methods.begin(); it != methods.end(); it++) {
		if ((*it)->get_name() == name) {
			if ((*it)->is_inherited() && (*it)->is_virtual()) {
				// Override the inherited virtual method
				methods.erase(it);
				methods.push_back(method);
				return true;
			}
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
	constructor_set = true;
	return true;
}

bool bpp_class::set_destructor(std::shared_ptr<bpp_destructor> destructor) {
	if (destructor_set) {
		return false;
	}
	this->destructor = destructor;
	destructor_set = true;
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

std::shared_ptr<bpp::bpp_method> bpp_class::get_method(const std::string& name, std::shared_ptr<bpp_entity> context) {
	for (auto& m : methods) {
		if (m->get_name() == name) {
			if (m->get_scope() == bpp_scope::SCOPE_PUBLIC) {
				return m;
			}

			if (m->get_scope() == bpp_scope::SCOPE_PRIVATE || m->get_scope() == bpp_scope::SCOPE_PROTECTED) {
				if (context == this->get_class()) {
					return m;
				} else {
					return bpp::inaccessible_method;
				}
			}

			if (m->get_scope() == bpp_scope::SCOPE_INACCESSIBLE) {
				return bpp::inaccessible_method;
			}
		}
	}
	return nullptr;
}

std::shared_ptr<bpp::bpp_datamember> bpp_class::get_datamember(const std::string& name, std::shared_ptr<bpp_entity> context) {
	for (auto& d : datamembers) {
		if (d->get_name() == name) {
			if (d->get_scope() == bpp_scope::SCOPE_PUBLIC) {
				return d;
			}

			if (d->get_scope() == bpp_scope::SCOPE_PRIVATE || d->get_scope() == bpp_scope::SCOPE_PROTECTED) {
				if (context == this->get_class()) {
					return d;
				} else {
					return bpp::inaccessible_datamember;
				}
			}

			if (d->get_scope() == bpp_scope::SCOPE_INACCESSIBLE) {
				return bpp::inaccessible_datamember;
			}
		}
	}
	return nullptr;
}

void bpp_class::inherit(std::shared_ptr<bpp_class> parent) {
	// Inherit methods
	for (auto& m : parent->get_methods()) {
		if (m->get_name() == "toPrimitive") {
			continue; // Don't inherit toPrimitive
		}
		// Write a proxy method to the base class's method
		std::shared_ptr<bpp_method> proxy = std::make_shared<bpp_method>();
		proxy->set_name(m->get_name());
		proxy->set_scope( (m->get_scope() == bpp_scope::SCOPE_PRIVATE) ? bpp_scope::SCOPE_INACCESSIBLE : m->get_scope() );
		proxy->set_virtual(m->is_virtual());
		proxy->set_inherited(true);

		std::string proxy_method_code = "bpp__" + parent->get_name() + "__" + m->get_name()
			+ " ${__objectAddress} 1";
		// Add the parameters
		for (auto& p : m->get_parameters()) {
			proxy->add_parameter(p);
			proxy_method_code += " \"$" + p->get_name() + "\"";
		}
		proxy_method_code += "\n";
		proxy->add_code(proxy_method_code);

		methods.push_back(proxy);
	}

	// Inherit datamembers
	for (auto& d : parent->get_datamembers()) {
		datamembers.push_back(d);
		// If the datamember is marked private, mark it as inaccessible
		if (d->get_scope() == bpp_scope::SCOPE_PRIVATE) {
			datamembers.back()->set_scope(bpp_scope::SCOPE_INACCESSIBLE);
		}
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

	// Inherit parents
	for (auto& p : parent->parents) {
		parents.push_back(p);
	}

	// Mark the parent as a parent of this class
	parents.push_back(parent);
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
