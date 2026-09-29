/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Object.h"

#include <IR/entities/Class.h>

#include <error/InternalError.h>

namespace bpp::IR {

Object::Object(const Object& other) : Entity(other), NamedEntity(other), AddressableEntity(other),
	m_is_pointer(other.m_is_pointer), type(other.type), copy_from(other.copy_from)
{
	// Because we own a unique_ptr to our initial value, the copy constructor for Object is implicitly deleted
	// However, this initial value will never be modified
	// So, in the copied object, we can just store a pointer to the original initial value, rather than copying it
	if (other.initial_value.has_value()) {
		if (std::holds_alternative<std::unique_ptr<CodeEntity>>(other.initial_value.value())) {
			initial_value = std::get<std::unique_ptr<CodeEntity>>(other.initial_value.value()).get();
		} else {
			initial_value = std::get<const CodeEntity*>(other.initial_value.value());
		}
	}
}

void Object::setInitialValue(std::unique_ptr<CodeEntity> value) {
	initial_value = std::move(value);
}

std::optional<const CodeEntity*> Object::getInitialValue() const {
	if (!initial_value.has_value()) return std::nullopt;
	if (std::holds_alternative<std::unique_ptr<CodeEntity>>(initial_value.value())) {
		return std::get<std::unique_ptr<CodeEntity>>(initial_value.value()).get();
	} else {
		return std::get<const CodeEntity*>(initial_value.value());
	}
}

std::unique_ptr<CodeEntity> Object::releaseInitialValue() {
	if (!initial_value.has_value()) return nullptr;
	if (std::holds_alternative<std::unique_ptr<CodeEntity>>(initial_value.value())) {
		auto result = std::move(std::get<std::unique_ptr<CodeEntity>>(initial_value.value()));
		initial_value.reset();
		return result;
	} else {
		return nullptr;
	}
}

std::string Object::getAddress() const {
	bpp_assert(type, "Object does not have a type");
	bpp_assert(!name.empty(), "Object does not have a name");

	std::string address = "bpp__";

	if (m_is_pointer) address += "__ptr__";

	address += type->getName() + "__" + name;

	return address;
}

PRETTYPRINT_IMPLEMENTATION(Object, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(";
	if (!type) {
		os << "Primitive";
	} else {
		os << type->getName();
	}
	if (m_is_pointer) os << "*";
	os << " " << name;
	if (initial_value.has_value()) {
		os << "\n" << indent << "  =\n";
		if (std::holds_alternative<std::unique_ptr<CodeEntity>>(initial_value.value())) {
			std::get<std::unique_ptr<CodeEntity>>(initial_value.value())->prettyPrint(os, indentation_level + 1);
		} else {
			std::get<const CodeEntity*>(initial_value.value())->prettyPrint(os, indentation_level + 1);
		}
		os << indent;
	} else if (copy_from != nullptr) {
		os << "\n" << indent << "  = copy of " << copy_from->getName() << "\n" << indent;
	}
	os << ")\n";
	return os;
})

} // namespace bpp::IR
