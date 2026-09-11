/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/NamedEntity.h>
#include <IR/entities/AddressableEntity.h>
#include <IR/entities/Class.h>
#include <IR/entities/CodeEntity.h>

#include <optional>

namespace bpp::IR {

/**
 * @brief An object in Bash++.
 *
 * This includes both non-primitives and pointers.
 * Whether the object is a pointer, as well as its type, must be given in the constructor.
 */
class Object : public Entity, public NamedEntity, public AddressableEntity {
	private:
		bool m_is_pointer = false;

		std::weak_ptr<const Class> type;

		// Initialization information:
		/// If a pointer or primitive, the initial value (if any)
		std::optional<std::shared_ptr<CodeEntity>> initial_value = std::nullopt;

		/// If not a pointer, the object from which this is copied (if any)
		std::shared_ptr<Object> copy_from = nullptr;
	public:
		std::string getAddress() const override;

		bool isPointer() const { return m_is_pointer; }
		void setIsPointer(bool is_pointer) { m_is_pointer = is_pointer; }

		std::weak_ptr<const Class> getType() const { return type; }
		void setType(std::weak_ptr<const Class> type) { this->type = std::move(type); }

		bool isPrimitive() const { return type.expired() || isPointer(); }

		void setInitialValue(const std::shared_ptr<CodeEntity>& value) { initial_value = value; }
		const std::optional<std::shared_ptr<CodeEntity>>& getInitialValue() const { return initial_value; }
		bool hasInitialValue() const { return initial_value.has_value(); }

		void setCopyFrom(std::shared_ptr<Object> other) { copy_from = std::move(other); }
		std::shared_ptr<Object> getCopyFrom() const { return copy_from; }

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(";
			if (type.expired()) {
				os << "Primitive";
			} else {
				os << type.lock()->getName();
			}
			if (m_is_pointer) os << "*";
			os << " " << name;
			if (initial_value.has_value()) {
				os << "\n" << indent << "  =\n";
				initial_value.value()->prettyPrint(os, indentation_level + 1);
				os << indent;
			} else if (copy_from != nullptr) {
				os << "\n" << indent << "  = copy of " << copy_from->getName() << "\n" << indent;
			}
			os << ")\n";
			return os;
		})
};

} // namespace bpp::IR
