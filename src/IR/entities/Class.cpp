/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Class.h"
#include "Method.h"
#include "DataMember.h"

#include <error/VisibilityError.h>

namespace bpp::IR {

/**
 * @brief Check if this class is derived from some other particular class
 * 
 * @param other The possible ancestor of this class
 * @return true If `other` is an ancestor (or immediate parent) of this class
 * @return false Otherwise
 */
bool Class::is_derived_from(std::shared_ptr<Class> other) const {
	auto parent = this->parent_class.lock();

	while (parent != nullptr) {
		if (parent == other) return true;
		parent = parent->get_parent_class();
	}

	return false;
}

void Class::inherit(std::shared_ptr<Class> parent) {
	// Inherit methods
	methods.reserve(methods.size() + parent->methods.size());
	for (const auto& m : parent->get_methods()) {
		if (m->get_name() == "toPrimitive") continue; // Don't inherit the toPrimitive method, since it is automatically generated for all classes
		auto inherited_method = std::make_shared<Method>(*m);
		if (inherited_method->get_scope() == VisibilityScope::PRIVATE) {
			inherited_method->set_scope(VisibilityScope::INACCESSIBLE);
		}
		inherited_method->set_is_inherited(true);
		inherited_method->set_parent_method(m);
		if (inherited_method->is_virtual()) inherited_method->set_is_overridable(true);
		add_method(inherited_method);
	}

	// Inherit data members
	datamembers.reserve(datamembers.size() + parent->datamembers.size());
	for (const auto& d : parent->get_datamembers()) {
		auto inherited_datamember = std::make_shared<DataMember>(*d);
		if (inherited_datamember->get_scope() == VisibilityScope::PRIVATE) {
			inherited_datamember->set_scope(VisibilityScope::INACCESSIBLE);
		}
		inherited_datamember->set_parent_datamember(d);
		add_datamember(inherited_datamember);
	}

	this->parent_class = parent;
}

bool Class::add_method(std::shared_ptr<Method> method) {
	for (auto it = methods.begin(); it != methods.end(); it++) {
		const auto& existing_method = *it;
		if (existing_method->get_name() != method->get_name()) continue; // Not the same name, so no conflict
		if (existing_method->is_overridable()) {
			methods.erase(it); // Remove the existing method, since it is being overridden
			method->set_is_overridable(false); // Can't override it twice
			method->set_is_inherited(false); // This is a new method, not inherited
			method->set_parent_method(existing_method->get_parent_method()); // Keep the chain of inheritance intact
			break;
		} else {
			return false; // Conflict with an existing method that is not overridable
		}
	}

	// If this method shares a name with a data member, that's an error
	for (const auto& d : datamembers) {
		if (d->get_name() == method->get_name()) return false;
	}

	method->set_containing_class(weak_from_this());

	methods.push_back(method);
	return true;
}

bool Class::add_datamember(std::shared_ptr<DataMember> datamember) {
	for (const auto& d : datamembers) {
		if (d->get_name() == datamember->get_name()) return false; // Conflict with an existing data member
	}

	// If this data member shares a name with a method, that's an error
	for (const auto& m : methods) {
		if (m->get_name() == datamember->get_name()) return false;
	}

	datamember->set_containing_class(weak_from_this());

	datamembers.push_back(datamember);
	return true;
}

template <ClassMember T>
std::shared_ptr<T> Class::get_member(const std::string& name, std::shared_ptr<Entity> context) {
	std::vector<std::shared_ptr<T>>* container;
	// The following static_assert is probably redundant since the concept ClassMember is restricted to one of those two types
	static_assert(std::is_same_v<T, Method> || std::is_same_v<T, DataMember>, "T must be either Method or DataMember");
	if constexpr (std::is_same_v<T, Method>) {
		container = &methods;
	} else if constexpr (std::is_same_v<T, DataMember>) {
		container = &datamembers;
	}

	for (const auto& m : *container) {
		if (m->get_name() != name) continue;

		switch (m->get_scope()) {
			case VisibilityScope::INACCESSIBLE: break; // Never OK
			case VisibilityScope::PUBLIC: return m; // Always OK
			case VisibilityScope::PRIVATE:
				if (context->get_containing_class().lock() == shared_from_this()) return m; // Only OK if the context is in precisely the same class
				break;
			case VisibilityScope::PROTECTED: {
				// OK if the context is in either this same class or a descendant (child) class
				auto possible_descendant = context->get_containing_class().lock();
				if (!possible_descendant) break; // Context is not in a class, so not OK
				if (possible_descendant == shared_from_this() || possible_descendant->is_derived_from(shared_from_this())) return m;
				break;
			}
		}

		// If we're here:
		// - The class was found
		// - Visibility rules denied access given the context
		throw bpp::ErrorHandling::VisibilityError();
	}

	return nullptr;
}

/**
 * @brief Get a method by name
 *
 * This returns a method by name, taking into account the visibility restrictions of the method
 *
 * If the method is public, it is always returned
 *
 * If the method is private, it can only be returned if the context is the same as the owning class
 *
 * If the method is proteted, it can only be returned if the context is the same as the owning class, or is a class derived from the owning class
 * 
 * @param name The name of the method to get
 * @param context The context from which the method is being requested
 * @return std::shared_ptr<Method> The method, or nullptr if it doesn't exist
 * @throws bpp::ErrorHandling::VisibilityError if the context does not permit access
 */
std::shared_ptr<Method> Class::get_method(const std::string& name, std::shared_ptr<Entity> context) {
	return get_member<Method>(name, context);
}

/**
 * @brief Get a method by name without checking the context against visibility rules
 *
 * This function is UNSAFE and should only be used when the context is known to be correct or the consequneces of an incorrect context are acceptable.
 * 
 * @param name The name of the method to get
 * @return std::shared_ptr<Method> The method, or nullptr if not found
 */
std::shared_ptr<Method> Class::get_method_UNSAFE(const std::string& name) {
	for (auto& method : methods) {
		if (method->get_name() == name) return method;
	}

	return nullptr;
}

/**
 * @brief Get a data member by name
 *
 * This returns a data member by name, taking into account the visibility restrictions of the data member
 *
 * If the data member is public, it is always returned
 *
 * If the data member is private, it can only be returned if the context is the same as the owning class
 *
 * If the data member is proteted, it can only be returned if the context is the same as the owning class, or is a class derived from the owning class
 * 
 * @param name The name of the data member to get
 * @param context The context from which the data member is being requested
 * @return std::shared_ptr<DataMember> The data member, or nullptr if it doesn't exist
 * @throws bpp::ErrorHandling::VisibilityError if the context does not permit access
 */
std::shared_ptr<DataMember> Class::get_datamember(const std::string& name, std::shared_ptr<Entity> context) {
	return get_member<DataMember>(name, context);
}

/**
 * @brief Get a data member by name without checking the context against visibility rules
 *
 * This function is UNSAFE and should only be used when the context is known to be correct or the consequneces of an incorrect context are acceptable.
 * 
 * @param name The name of the data member to get
 * @return std::shared_ptr<DataMember> The data member, or nullptr if not found
 */
std::shared_ptr<DataMember> Class::get_datamember_UNSAFE(const std::string& name) {
	for (auto& datamember : datamembers) {
		if (datamember->get_name() == name) return datamember;
	}

	return nullptr;
}

bpp::CodeGen::CodeSegment Class::generate_code() {
	bpp::CodeGen::CodeSegment code;

	for (const auto& method : methods) {
		code.egalitarian_merge(method->generate_code());
	}

	return code;
}

PRETTYPRINT_IMPLEMENTATION(Class, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(Class " << name << "\n";

	for (const auto& datamember : datamembers) {
		datamember->prettyPrint(os, indentation_level + 1);
	}
	
	for (const auto& method : methods) {
		method->prettyPrint(os, indentation_level + 1);
	}

	os << indent << ")\n";
	return os;
})


} // namespace bpp::IR
