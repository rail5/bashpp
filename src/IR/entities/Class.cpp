/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Class.h"
#include <IR/entities/Method.h>
#include <IR/entities/MethodParameter.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/expressions/bpp/DynamicCast.h>
#include <IR/entities/Program.h>
#include <IR/SourceLocation.h>

#include <error/InternalError.h>

#include <vector>
#include <ranges>

namespace bpp::IR {

void Class::initSpecialPointers() const {
	if (this_ptr) return;

	this_ptr = std::make_unique<ThisPtr>(this);
	this_ptr->setContainingProgram(getContainingProgram());

	// Set initial value: dynamic cast of $1 to this class type
	auto dynamic_cast_entity = std::make_unique<DynamicCast>();
	dynamic_cast_entity->inherit(this_ptr.get());
	dynamic_cast_entity->setTargetType(getName());
	dynamic_cast_entity->add("$1");
	this_ptr->setInitialValue(std::move(dynamic_cast_entity));

	if (parent_class) super_ptr = std::make_unique<ThisPtr>(parent_class, "super");
}

bool Class::isDerivedFrom(const Class* other) const {
	for (const auto* parent = this->parent_class; parent != nullptr; parent = parent->getParentClass()) {
		if (parent == other) return true;
	}

	return false;
}

void Class::inherit(const Class* parent) {
	// Inherit methods
	methods.reserve(methods.size() + parent->methods.size());
	bpp_assert(getContainingProgram(), "Class has no containing program");
	for (const auto& m : parent->getAllMethods()) {
		if (m->getName() == "toPrimitive" && m->isDefaulted()) continue; // Only inherit custom overrides of toPrimitive
		if (m->getName().starts_with("__")) continue; // Don't inherit system methods, since they are automatically generated for all classes
		auto inherited_method = std::make_unique<Method>(m.get());
		if (!addMethod(std::move(inherited_method))) {
			throw bpp::ErrorHandling::InternalError("Failed to inherit method '" + m->getName() + "' from parent class '" + parent->getName() + "'");
		}
	}

	// Inherit data members
	datamembers.reserve(datamembers.size() + parent->datamembers.size());
	for (const auto& d : parent->getAllDatamembers()) {
		auto inherited_datamember = std::make_unique<DataMember>(*d);
		if (inherited_datamember->getScope() == VisibilityScope::PRIVATE) {
			inherited_datamember->setScope(VisibilityScope::INACCESSIBLE);
		}
		inherited_datamember->setParentDatamember(d.get());
		if (!addDatamember(std::move(inherited_datamember))) {
			throw bpp::ErrorHandling::InternalError("Failed to inherit data member '" + d->getName() + "' from parent class '" + parent->getName() + "'");
		}
	}

	this->parent_class = parent;
}

std::expected<const Method*, NameConflictError> Class::addMethod(std::unique_ptr<Method> method) {
	preregistered_method = nullptr;

	if (auto* existing_method = getMethod_UNSAFE(method->getName())) {
		if (!existing_method->isOverridable()) return std::unexpected(NameConflictError::EXISTING_METHOD); // Not overridable, so can't override it

		// Otherwise: override
		const auto* parent_method = existing_method->getParentMethod();
		bool wasVirtual = existing_method->isVirtual();
		const std::deque<SourceLocationRange> reference_positions = existing_method->getReferencePositions(); // copy

		*existing_method = std::move(*method);

		existing_method->setParentMethod(parent_method); // Keep the chain of inheritance intact
		existing_method->setIsOverridable(false); // Can't override it twice
		existing_method->setIsVirtual(wasVirtual);
		existing_method->setContainingClass(this);
		for (const auto& ref : reference_positions) {
			existing_method->addReferencePosition({ref.source_file, ref.start});
		}

		return existing_method;
	}

	// If this method shares a name with a data member, that's an error
	if (getDatamember_UNSAFE(method->getName())) return std::unexpected(NameConflictError::EXISTING_DATAMEMBER);

	method->setContainingClass(this);

	return methods.emplace_back(std::move(method)).get();
}

void Class::preregisterMethod(Method* method) {
	bpp_assert(method != nullptr, "Method pointer is null");
	method->setContainingClass(this);
	preregistered_method = method;
}

std::expected<const DataMember*, NameConflictError> Class::addDatamember(std::unique_ptr<DataMember> datamember) {
	if (getDatamember_UNSAFE(datamember->getName())) return std::unexpected(NameConflictError::EXISTING_DATAMEMBER);
	if (getMethod_UNSAFE(datamember->getName())) return std::unexpected(NameConflictError::EXISTING_METHOD);

	datamember->setContainingClass(this);

	datamembers.push_back(std::move(datamember));
	return datamembers.back().get();
}

template <ClassMember T>
std::expected<T*, LookupError> Class::getMember(const std::string& name, const Entity* context) const {
	const std::vector<std::unique_ptr<T>>* container = nullptr;
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
			case VisibilityScope::PUBLIC: return m.get(); // Always OK
			case VisibilityScope::PRIVATE:
				if (context->getContainingClass() == this) return m.get(); // Only OK if the context is in precisely the same class
				break;
			case VisibilityScope::PROTECTED: {
				// OK if the context is in either this same class or a descendant (child) class
				const auto* possible_descendant = context->getContainingClass();
				if (!possible_descendant) break; // Context is not in a class, so not OK
				if (possible_descendant == this || possible_descendant->isDerivedFrom(this)) return m.get();
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

std::expected<Method*, LookupError> Class::getMethod(const std::string& name, const Entity* context) const {
	auto res = getMember<Method>(name, context);
	if (!res && res.error() == LookupError::NOT_FOUND) {
		// If not found in the class itself, check preregistered methods
		if (preregistered_method && preregistered_method->getName() == name) return preregistered_method;
	}
	return res;
}

Method* Class::getMethod_UNSAFE(const std::string& name) const {
	for (const auto& method : methods) {
		if (method->getName() == name) return method.get();
	}

	// If not found in the class itself, check preregistered methods
	if (preregistered_method && preregistered_method->getName() == name) return preregistered_method;

	return nullptr;
}

std::expected<DataMember*, LookupError> Class::getDatamember(const std::string& name, const Entity* context) const {
	return getMember<DataMember>(name, context);
}

DataMember* Class::getDatamember_UNSAFE(const std::string& name) const {
	for (const auto& datamember : datamembers) {
		if (datamember->getName() == name) return datamember.get();
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
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment code;

	state->current_class = this;

	code.add_post_code("declare -A bpp__" + getName() + "____vTable\n");
	if (const auto* parent = getParentClass()) {
		code.add_post_code("bpp__" + getName() + R"(____vTable["__parent__"]="bpp__)" + parent->getName() + "____vTable\"\n");
	}

	for (const auto& method : methods) {
		code.absorb_all_to_main(method->generateCode(state));
		if (method->isVirtual()) {
			code.add_post_code("bpp__" + getName() + "____vTable[\"" + method->getName() + "\"]=\"" + method->getAddress() +"\"\n");
		}
	}

	state->current_class = nullptr;

	return code;
}

bpp::CodeGen::CodeSegment Class::callConstructors(bpp::CodeGen::CodeGenState* state, const std::string& object_address) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(!object_address.empty(), "Object address is empty");
	bpp::CodeGen::CodeSegment code;

	std::vector<const Class*> inheritance_chain;
	for (const auto* cls = this; cls != nullptr; cls = cls->getParentClass()) {
		inheritance_chain.push_back(cls);
	}

	// Call constructors in parent -> child order
	// (Base constructor first, derived constructor last)
	for (const auto* cls : std::ranges::reverse_view(inheritance_chain)) {
		const auto* constructor = cls->getMethod_UNSAFE("__constructor");
		if (constructor) code.add_main_code(constructor->getAddress() + " " + object_address + "\n");
	}

	return code;
}

bool Class::hasConstructors() const {
	for (const auto* cls = this; cls != nullptr; cls = cls->getParentClass()) {
		if (cls->getMethod_UNSAFE("__constructor")) return true;
	}
	return false;
}

bpp::CodeGen::CodeSegment Class::callDestructors(bpp::CodeGen::CodeGenState* state, const std::string& object_address) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(!object_address.empty(), "Object address is empty");
	bpp::CodeGen::CodeSegment code;

	// Call destructors in child -> parent order
	// (Derived destructor first, base destructor last)
	for (const auto* cls = this; cls != nullptr; cls = cls->getParentClass()) {
		const auto* destructor = cls->getMethod_UNSAFE("__destructor");
		if (destructor) code.add_main_code(destructor->getAddress() + " " + object_address + "\n");
	}

	return code;
}

bool Class::hasDestructors() const {
	for (const auto* cls = this; cls != nullptr; cls = cls->getParentClass()) {
		if (cls->getMethod_UNSAFE("__destructor")) return true;
	}
	return false;
}

PRETTYPRINT_IMPLEMENTATION(Class, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(Class " << name;
	if (auto parent = getParentClass()) {
		os << " : " << parent->getName();
	}
	os << "\n";

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
