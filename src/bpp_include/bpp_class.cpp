/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_CLASS_CPP_
#define SRC_BPP_INCLUDE_BPP_CLASS_CPP_

#include "bpp.h"
#include "bpp_codegen.h"

namespace bpp {

bpp_class::bpp_class() {
	add_default_destructor();
}

/**
 * @brief Remove the default toPrimitive method
 * 
 * Each class has a default toPrimitive method which simply echoes the class name.
 * This method is only added if the class does not have a custom toPrimitive method.
 */
void bpp_class::remove_default_toPrimitive()  {
	if (!has_custom_toPrimitive) {
		// Remove the toPrimitive method from the methods vector
		for (auto it = methods.begin(); it != methods.end(); it++) {
			if ((*it)->get_name() == "toPrimitive") {
				methods.erase(it);
				break;
			}
		}
	}
}

/**
 * @brief Add the default toPrimitive method
 * 
 * Each class has a default toPrimitive method which simply echoes the class name.
 * This method is only added if the class does not have a custom toPrimitive method.
 */
void bpp_class::add_default_toPrimitive()  {
	if (!has_custom_toPrimitive) {
		std::shared_ptr<bpp_method> toPrimitive = std::make_shared<bpp_method>();
		toPrimitive->set_name("toPrimitive");
		std::string default_toPrimitive_body = "	echo " + name + " Instance\n";
		toPrimitive->add_code(default_toPrimitive_body);
		toPrimitive->set_scope(bpp_scope::SCOPE_PUBLIC);
		toPrimitive->set_virtual(true);
		remove_default_toPrimitive();
		methods.push_back(toPrimitive);
	}
}

/**
 * @brief Remove the default destructor method.
 */
void bpp_class::remove_default_destructor()  {
	if (!has_custom_destructor) {
		// Remove the destructor method from the methods vector
		for (auto it = methods.begin(); it != methods.end(); it++) {
			if ((*it)->get_name() == "__destructor") {
				methods.erase(it);
				break;
			}
		}
	}
}

/**
 * @brief Add the default destructor method.
 * 
 * The default destructor does nothing.
 * The reason we add it is to ensure that a destructor is always present, even if the user does not define one.
 * This is important because destructors are called automatically in some cases, such as when an object goes out of scope.
 */
void bpp_class::add_default_destructor()  {
	if (!has_custom_destructor) {
		std::shared_ptr<bpp_method> destructor = std::make_shared<bpp_method>();
		destructor->set_name("__destructor");
		destructor->set_scope(bpp_scope::SCOPE_PUBLIC);
		destructor->set_virtual(true);
		remove_default_destructor();
		methods.push_back(destructor);
	}
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

/**
 * @brief Add a method to the class
 */
bool bpp_class::add_method(std::shared_ptr<bpp_method> method) {
	std::string name = method->get_name();

	if (name == "toPrimitive" && !has_custom_toPrimitive) {
		// toPrimitive must ALWAYS be public
		if (method->get_scope() != bpp_scope::SCOPE_PUBLIC) {
			return false;
		}
		method->set_virtual(true);
		remove_default_toPrimitive();
		has_custom_toPrimitive = true;
	}

	if (name == "__destructor" && !has_custom_destructor) {
		method->set_virtual(true);
		remove_default_destructor();
		has_custom_destructor = true;
	}

	for (auto it = methods.begin(); it != methods.end(); it++) {
		if ((*it)->get_name() == name) {
			if ((*it)->is_inherited() && (*it)->is_virtual()) {
				// Override the inherited virtual method
				method->set_virtual(true);
				methods.erase(it);
				methods.push_back(method);
				return true;
			}
			return false;
		}
	}

	// If this method shares the name of a datamember, reject it
	for (auto& d : datamembers) {
		if (d->get_name() == name) {
			return false;
		}
	}

	methods.push_back(method);
	return true;
}

/**
 * @brief Add a datamember to the class
 */
bool bpp_class::add_datamember(std::shared_ptr<bpp_datamember> datamember) {
	std::string name = datamember->get_name();
	for (auto& d : datamembers) {
		if (d->get_name() == name) {
			return false;
		}
	}

	// If this datamember shares the name of a method, reject it
	for (auto& m : methods) {
		if (m->get_name() == name) {
			return false;
		}
	}

	datamembers.push_back(datamember);
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

/**
 * @brief Get a method by name
 * 
 * This function returns a method by name, taking into account the scope of the method.
 * 
 * If the method is public, it is returned.
 * 
 * If the method is private or protected, it is returned only if the context permits. Otherwise, we return bpp::inaccessible_method.
 * 
 * @param name The name of the method to get
 * @param context The context in which the method is being accessed
 * @return The method, bpp::inaccessible_method, or nullptr if it does not exist
 */
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

/**
 * @brief Get a method by name without checking the context
 * 
 * This function returns a method by name without checking the context.
 * 
 * This function is UNSAFE and should only be used when the context is known to be correct or the consequences of an incorrect context are acceptable.
 * 
 * @param name The name of the method to get
 * @return The method, or nullptr if it does not exist
 */
std::shared_ptr<bpp::bpp_method> bpp_class::get_method_UNSAFE(const std::string& name) {
	for (auto& m : methods) {
		if (m->get_name() == name) {
			return m;
		}
	}
	return nullptr;
}

/**
 * @brief Get a datamember by name
 * 
 * This function returns a datamember by name, taking into account the scope of the datamember.
 * 
 * If the datamember is public, it is returned.
 * 
 * If the datamember is private or protected, it is returned only if the context permits. Otherwise, we return bpp::inaccessible_datamember.
 * 
 * @param name The name of the datamember to get
 * @param context The context in which the datamember is being accessed
 * @return The datamember, bpp::inaccessible_datamember, or nullptr if it does not exist
 */
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

/**
 * @brief Inherit from a parent class
 * 
 * This function copies all methods and datamembers from the parent class into this class.
 * 
 * @param parent The parent class to inherit from
 */
void bpp_class::inherit(std::shared_ptr<bpp_class> parent) {
	// Inherit methods
	for (auto& m : parent->get_methods()) {
		if (m->get_name() == "toPrimitive" || m->get_name() == "__delete") {
			continue; // Don't inherit toPrimitive or system methods
		}
		// Write a proxy method to the base class's method
		std::shared_ptr<bpp_method> proxy = std::make_shared<bpp_method>();
		proxy->set_name(m->get_name());
		proxy->set_scope( (m->get_scope() == bpp_scope::SCOPE_PRIVATE) ? bpp_scope::SCOPE_INACCESSIBLE : m->get_scope() );
		proxy->set_virtual(m->is_virtual());
		proxy->set_inherited(true);

		std::string proxy_method_code = "bpp__" + parent->get_name() + "__" + m->get_name()
			+ " ${__objectAddress}";
		// Add the parameters
		for (auto& p : m->get_parameters()) {
			proxy->add_parameter(p);
			proxy_method_code += " \"$" + p->get_name() + "\"";
		}
		proxy_method_code += "\n";
		proxy->add_code(proxy_method_code);

		add_method(proxy);
	}

	// Inherit datamembers
	for (auto& d : parent->get_datamembers()) {
		datamembers.push_back(d);
		// If the datamember is marked private, mark it as inaccessible
		if (d->get_scope() == bpp_scope::SCOPE_PRIVATE) {
			datamembers.back()->set_scope(bpp_scope::SCOPE_INACCESSIBLE);
		}
	}

	// Inherit parents
	for (auto& p : parent->parents) {
		parents.push_back(p);
	}

	// Mark the parent as a parent of this class
	parents.push_back(parent);
}

std::shared_ptr<bpp::bpp_class> bpp_class::get_parent() {
	if (parents.size() == 0) {
		return nullptr;
	}
	return parents.back();
}

void bpp_class::finalize(std::shared_ptr<bpp_program> program) {
	if (finalized) {
		return;
	}

	// Now that we're certain there won't be any more methods or data members added
	// Generate the system __delete method
	std::shared_ptr<bpp_method> delete_method = std::make_shared<bpp_method>();
	delete_method->set_name("__delete");
	delete_method->set_scope(bpp_scope::SCOPE_PUBLIC);
	delete_method->set_virtual(true);
	
	for (auto& dm : datamembers) {
		if (dm->get_class()->get_name() == "primitive" || dm->is_pointer()) {
			delete_method->add_code("	unset ${__objectAddress}__" + dm->get_name() + "\n");
		} else {
			code_segment delete_code = generate_delete_code(dm, "${__objectAddress}__" + dm->get_name(), program);
			delete_method->add_code(delete_code.pre_code + "\n");
			delete_method->add_code("	unset ${__objectAddress}__" + dm->get_name() + "\n");
		}
		delete_method->add_code(dm->get_post_access_code() + "\n");
	}

	// Add the delete method to the class
	add_method(delete_method);
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CLASS_CPP
