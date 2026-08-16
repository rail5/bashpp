/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Class.h"
#include "Method.h"
#include "MethodParameter.h"
#include "DataMember.h"

#include <error/InternalError.h>

namespace bpp::IR {

void Class::init_special_pointers() const {
	if (special_pointers_initialized) return;

	this_ptr = std::make_shared<ThisPtr>(shared_from_this());
	this_ptr->set_containing_program(containing_program);

	if (auto parent = parent_class.lock()) {
		super_ptr = std::make_shared<ThisPtr>(parent);
		super_ptr->set_name("super");
	}

	special_pointers_initialized = true;
}

bool Class::is_derived_from(std::shared_ptr<const Class> other) const {
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
		if (m->get_name().starts_with("__")) continue; // Don't inherit system methods, since they are automatically generated for all classes
		auto inherited_method = std::make_shared<Method>(*m);
		if (inherited_method->get_scope() == VisibilityScope::PRIVATE) {
			inherited_method->set_scope(VisibilityScope::INACCESSIBLE);
		}
		inherited_method->set_is_inherited(true);
		inherited_method->set_parent_method(m);
		if (inherited_method->is_virtual()) inherited_method->set_is_overridable(true);
		if (!add_method(std::move(inherited_method))) {
			throw bpp::ErrorHandling::InternalError("Failed to inherit method '" + m->get_name() + "' from parent class '" + parent->get_name() + "'");
		}
	}

	// Inherit data members
	datamembers.reserve(datamembers.size() + parent->datamembers.size());
	for (const auto& d : parent->get_datamembers()) {
		auto inherited_datamember = std::make_shared<DataMember>(*d);
		if (inherited_datamember->get_scope() == VisibilityScope::PRIVATE) {
			inherited_datamember->set_scope(VisibilityScope::INACCESSIBLE);
		}
		inherited_datamember->set_parent_datamember(d);
		if (!add_datamember(inherited_datamember)) {
			throw bpp::ErrorHandling::InternalError("Failed to inherit data member '" + d->get_name() + "' from parent class '" + parent->get_name() + "'");
		}
	}

	this->parent_class = parent;
}

std::expected<std::shared_ptr<Method>, AddError> Class::add_method(std::shared_ptr<Method>&& method) {
	if (auto existing_method = get_method_UNSAFE(method->get_name())) {
		if (!existing_method->is_overridable()) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_METHOD); // Not overridable, so can't override it

		// Otherwise: override
		auto parent_method = existing_method->get_parent_method();

		*existing_method = std::move(*method);

		existing_method->set_parent_method(parent_method); // Keep the chain of inheritance intact
		existing_method->set_is_overridable(false); // Can't override it twice
		existing_method->set_is_inherited(false); // This is a new method, not inherited
		existing_method->set_containing_class(weak_from_this());

		return existing_method;
	}

	// If this method shares a name with a data member, that's an error
	if (get_datamember_UNSAFE(method->get_name())) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_DATAMEMBER);

	method->set_containing_class(weak_from_this());

	methods.emplace_back(std::move(method));
	return methods.back();
}

std::expected<void, AddError> Class::add_datamember(std::shared_ptr<DataMember> datamember) {
	if (get_datamember_UNSAFE(datamember->get_name())) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_DATAMEMBER);
	if (get_method_UNSAFE(datamember->get_name())) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_METHOD);

	datamember->set_containing_class(weak_from_this());

	datamembers.push_back(datamember);
	return {};
}

template <ClassMember T>
std::expected<std::shared_ptr<T>, LookupError> Class::get_member(const std::string& name, std::shared_ptr<Entity> context) const {
	const std::vector<std::shared_ptr<T>>* container = nullptr;
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
		return std::unexpected(LookupError::INACCESSIBLE);
	}

	return std::unexpected(LookupError::NOT_FOUND);
}

std::expected<std::shared_ptr<Method>, LookupError> Class::get_method(const std::string& name, std::shared_ptr<Entity> context) const {
	return get_member<Method>(name, context);
}

std::shared_ptr<Method> Class::get_method_UNSAFE(const std::string& name) const {
	for (const auto& method : methods) {
		if (method->get_name() == name) return method;
	}

	return nullptr;
}

std::expected<std::shared_ptr<DataMember>, LookupError> Class::get_datamember(const std::string& name, std::shared_ptr<Entity> context) const {
	return get_member<DataMember>(name, context);
}

std::shared_ptr<DataMember> Class::get_datamember_UNSAFE(const std::string& name) const {
	for (const auto& datamember : datamembers) {
		if (datamember->get_name() == name) return datamember;
	}

	return nullptr;
}

bpp::CodeGen::CodeSegment Class::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Class::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	for (const auto& method : methods) {
		code.absorb_all_to_main(method->generate_code(state));
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
