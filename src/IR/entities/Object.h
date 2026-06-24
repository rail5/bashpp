/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
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
class Object : public Entity {
	protected:
		std::string name;
		bool m_is_pointer = false;

		std::weak_ptr<Class> type;

		// Initialization information:
		/// If a pointer, the initial value of the pointer (if any)
		std::optional<std::shared_ptr<CodeEntity>> initial_value = std::nullopt;

		/// If not a pointer, the object from which this is copied (if any)
		std::shared_ptr<Object> copy_from = nullptr;

		// For codegen:
		std::string address;
	public:
		const std::string& get_name() const { return name; }
		void set_name(const std::string& name) { this->name = name; }

		bool is_pointer() const { return m_is_pointer; }
		void set_is_pointer(bool is_pointer) { m_is_pointer = is_pointer; }

		std::weak_ptr<Class> get_type() const { return type; }
		void set_type(std::weak_ptr<Class> type) { this->type = type; }

		void set_initial_value(const std::shared_ptr<CodeEntity>& value) { initial_value = value; }
		const std::optional<std::shared_ptr<CodeEntity>>& get_initial_value() const { return initial_value; }
		bool has_initial_value() const { return initial_value.has_value(); }

		void set_copy_from(std::shared_ptr<Object> other) { copy_from = other; }
		std::shared_ptr<Object> get_copy_from() const { return copy_from; }

		bpp::CodeGen::CodeSegment generate_code() override;
		
		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * 4, ' ');
			os << indent << "(";
			if (type.expired()) {
				os << "Primitive";
			} else {
				os << type.lock()->get_name();
			}
			if (m_is_pointer) os << "*";
			os << " " << name;
			if (initial_value.has_value()) {
				os << "\n" << indent << "  =\n";
				initial_value.value()->prettyPrint(os, indentation_level + 1);
			}
			if (copy_from != nullptr) {
				os << "\n" << indent << "  = copy of " << copy_from->get_name() << "\n";
			}
			os <<")\n";
			return os;
		})
};

} // namespace bpp::IR
