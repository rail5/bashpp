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

std::ostream& Class::prettyPrint(std::ostream& os, size_t indentation_level) const {
	std::string indent(indentation_level * 4, ' ');
	os << indent << "(Class " << name << "\n";

	for (const auto& datamember : datamembers) {
		datamember->prettyPrint(os, indentation_level + 1);
	}
	
	for (const auto& method : methods) {
		method->prettyPrint(os, indentation_level + 1);
	}

	os << indent << ")\n";
	return os;
}


} // namespace bpp::IR
