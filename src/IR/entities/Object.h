/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/Class.h>

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
		/// If a pointer, the string value to which this is initialized (if any)
		std::optional<std::string> initial_value = std::nullopt;

		/// If not a pointer, the object from which this is copied (if any)
		std::shared_ptr<Object> copy_from = nullptr;
	public:
		Object() = delete;
		Object(std::weak_ptr<Class> type, bool is_pointer) : m_is_pointer(is_pointer), type(type) {}

		const std::string& get_name() const { return name; }
		void set_name(const std::string& name) { this->name = name; }

		bool is_pointer() const { return m_is_pointer; }
		std::weak_ptr<Class> get_type() const { return type; }

		void set_initial_value(const std::string& value) { initial_value = value; }
		const std::optional<std::string>& get_initial_value() const { return initial_value; }
		bool has_initial_value() const { return initial_value.has_value(); }

		void set_copy_from(std::shared_ptr<Object> other) { copy_from = other; }
		std::shared_ptr<Object> get_copy_from() const { return copy_from; }
		
		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override {
			std::string indent(indentation_level * 4, ' ');
			os << indent << "(Object: " << name
				<< ", type: " << (type.expired() ? "null" : type.lock()->get_name())
				<< ", is_pointer: " << (m_is_pointer ? "true" : "false");
			if (initial_value.has_value()) {
				os << ", initial_value: \"" << initial_value.value() << "\"";
			}
			if (copy_from != nullptr) {
				os << ", copy_from: " << copy_from->get_name();
			}
			os << ")\n";
			return os;
		}
};

} // namespace bpp::IR
