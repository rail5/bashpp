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

void Class::initSpecialPointers() const {
	if (special_pointers_initialized) return;

	this_ptr = std::make_shared<ThisPtr>(shared_from_this());
	this_ptr->setContainingProgram(getContainingProgram());

	if (auto parent = parent_class.lock()) {
		super_ptr = std::make_shared<ThisPtr>(parent);
		super_ptr->setName("super");
	}

	special_pointers_initialized = true;
}

bool Class::isDerivedFrom(std::shared_ptr<const Class> other) const {
	auto parent = this->parent_class.lock();

	while (parent != nullptr) {
		if (parent == other) return true;
		parent = parent->getParentClass();
	}

	return false;
}

void Class::inherit(std::shared_ptr<const Class> parent) {
	// Inherit methods
	methods.reserve(methods.size() + parent->methods.size());
	for (const auto& m : parent->getAllMethods()) {
		if (m->getName() == "toPrimitive") continue; // Don't inherit the toPrimitive method, since it is automatically generated for all classes
		if (m->getName().starts_with("__")) continue; // Don't inherit system methods, since they are automatically generated for all classes
		auto inherited_method = std::make_shared<Method>(*m);
		if (inherited_method->getScope() == VisibilityScope::PRIVATE) {
			inherited_method->setScope(VisibilityScope::INACCESSIBLE);
		}
		inherited_method->setParentMethod(m);
		if (inherited_method->isVirtual()) inherited_method->setIsOverridable(true);
		if (!addMethod(std::move(inherited_method))) {
			throw bpp::ErrorHandling::InternalError("Failed to inherit method '" + m->getName() + "' from parent class '" + parent->getName() + "'");
		}
	}

	// Inherit data members
	datamembers.reserve(datamembers.size() + parent->datamembers.size());
	for (const auto& d : parent->getAllDatamembers()) {
		auto inherited_datamember = std::make_shared<DataMember>(*d);
		if (inherited_datamember->getScope() == VisibilityScope::PRIVATE) {
			inherited_datamember->setScope(VisibilityScope::INACCESSIBLE);
		}
		inherited_datamember->setParentDatamember(d);
		if (!addDatamember(inherited_datamember)) {
			throw bpp::ErrorHandling::InternalError("Failed to inherit data member '" + d->getName() + "' from parent class '" + parent->getName() + "'");
		}
	}

	this->parent_class = parent;
}

std::expected<std::shared_ptr<Method>, AddError> Class::addMethod(std::shared_ptr<Method>&& method) {
	if (auto existing_method = getMethod_UNSAFE(method->getName())) {
		if (!existing_method->isOverridable()) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_METHOD); // Not overridable, so can't override it

		// Otherwise: override
		auto parent_method = existing_method->getParentMethod();

		*existing_method = std::move(*method);

		existing_method->setParentMethod(parent_method); // Keep the chain of inheritance intact
		existing_method->setIsOverridable(false); // Can't override it twice
		existing_method->setContainingClass(weak_from_this());

		return existing_method;
	}

	// If this method shares a name with a data member, that's an error
	if (getDatamember_UNSAFE(method->getName())) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_DATAMEMBER);

	method->setContainingClass(weak_from_this());

	methods.emplace_back(std::move(method));
	return methods.back();
}

std::expected<void, AddError> Class::addDatamember(std::shared_ptr<DataMember> datamember) {
	if (getDatamember_UNSAFE(datamember->getName())) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_DATAMEMBER);
	if (getMethod_UNSAFE(datamember->getName())) return std::unexpected(AddError::NAME_CONFLICTS_WITH_EXISTING_METHOD);

	datamember->setContainingClass(weak_from_this());

	datamembers.push_back(datamember);
	return {};
}

template <ClassMember T>
std::expected<std::shared_ptr<T>, LookupError> Class::getMember(const std::string& name, std::shared_ptr<const Entity> context) const {
	const std::vector<std::shared_ptr<T>>* container = nullptr;
	// The following static_assert is probably redundant since the concept ClassMember is restricted to one of those two types
	static_assert(std::is_same_v<T, Method> || std::is_same_v<T, DataMember>, "T must be either Method or DataMember");
	if constexpr (std::is_same_v<T, Method>) {
		container = &methods;
	} else if constexpr (std::is_same_v<T, DataMember>) {
		container = &datamembers;
	}

	for (const auto& m : *container) {
		if (m->getName() != name) continue;

		switch (m->getScope()) {
			case VisibilityScope::INACCESSIBLE: break; // Never OK
			case VisibilityScope::PUBLIC: return m; // Always OK
			case VisibilityScope::PRIVATE:
				if (context->getContainingClass().lock() == shared_from_this()) return m; // Only OK if the context is in precisely the same class
				break;
			case VisibilityScope::PROTECTED: {
				// OK if the context is in either this same class or a descendant (child) class
				auto possible_descendant = context->getContainingClass().lock();
				if (!possible_descendant) break; // Context is not in a class, so not OK
				if (possible_descendant == shared_from_this() || possible_descendant->isDerivedFrom(shared_from_this())) return m;
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

std::expected<std::shared_ptr<Method>, LookupError> Class::getMethod(const std::string& name, std::shared_ptr<const Entity> context) const {
	return getMember<Method>(name, context);
}

std::shared_ptr<Method> Class::getMethod_UNSAFE(const std::string& name) const {
	for (const auto& method : methods) {
		if (method->getName() == name) return method;
	}

	return nullptr;
}

std::expected<std::shared_ptr<DataMember>, LookupError> Class::getDatamember(const std::string& name, std::shared_ptr<const Entity> context) const {
	return getMember<DataMember>(name, context);
}

std::shared_ptr<DataMember> Class::getDatamember_UNSAFE(const std::string& name) const {
	for (const auto& datamember : datamembers) {
		if (datamember->getName() == name) return datamember;
	}

	return nullptr;
}

bool Class::containsNonprimitiveDatamembers() const {
	for (const auto& dm : datamembers) {
		if (!dm->isPrimitive()) return true;
	}
	return false;
}

bpp::CodeGen::CodeSegment Class::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Class::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	state->current_class = shared_from_this();

	for (const auto& method : methods) {
		code.absorb_all_to_main(method->generateCode(state));
	}

	state->current_class = nullptr;

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
