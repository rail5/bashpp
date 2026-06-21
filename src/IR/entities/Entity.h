/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <list>
#include <string>
#include <memory>

#include <IR/bpp.h>

namespace bpp::IR {

/**
 * @brief The base class for all entities in Bash++
 *
 * All constructs in Bash++ are entities. This includes classes, methods, objects, and even the program itself.
 * It also includes constructs such as compound statements (e.g., 'if' statements and 'while' loops), although these entities don't have names.
 */
class Entity {
	protected:
		size_t parent_visible_object_count_at_creation = 0;
		size_t program_visible_class_count_at_creation = 0;

		/// The entity from which this entity inherits (applies to all entities except Program)
		std::weak_ptr<Entity> parent_entity;

		/// If this entity is inside of a class definition, this points to that class. Otherwise, it is null.
		std::weak_ptr<Class> containing_class;

		/// The program that this entity belongs to (only null for the Program entity itself)
		std::weak_ptr<Program> containing_program;

		/// Where in the source this entity was defined (used for error reporting / language server features)
		SymbolPosition definition_position;

		/// A list of all positions where this entity is referenced in the source (used for language server features)
		std::list<SymbolPosition> reference_positions;
	public:
		Entity() = default;
		virtual ~Entity() = default;

		Entity(const Entity& other) = default;
		Entity& operator=(const Entity& other) = default;
		Entity(Entity&& other) = default;
		Entity& operator=(Entity&& other) = default;

		virtual std::weak_ptr<Class> get_containing_class() { return containing_class; }
		void set_containing_class(std::weak_ptr<Class> containing_class) { this->containing_class = containing_class; }

		std::weak_ptr<Program> get_containing_program() const { return containing_program; }
		void set_containing_program(std::weak_ptr<Program> containing_program) { this->containing_program = containing_program; }

		SymbolPosition get_definition_position() const { return definition_position; }
		void set_definition_position(const SymbolPosition& pos) { this->definition_position = pos; }

		const std::list<SymbolPosition>& get_reference_positions() const { return reference_positions; }

		// Note: Methods require a different procedure.
		// Adding a reference to a derived class's version of an inherited method should also add a reference
		// to the base class's version of the method, since both are considered "used" in that case.
		virtual void add_reference_position(const SymbolPosition& pos) { this->reference_positions.push_back(pos); }

		void inherit(std::shared_ptr<Entity> parent);
		void inherit(std::shared_ptr<Program> program);

		virtual std::shared_ptr<Class> get_class(const std::string& name, size_t max_visible_index = SIZE_MAX);
		virtual std::shared_ptr<Object> get_object(const std::string& name, size_t max_visible_index = SIZE_MAX);

		virtual std::vector<std::shared_ptr<Class>> get_all_known_classes() const;
		virtual std::vector<std::shared_ptr<Object>> get_all_known_objects() const;

		virtual size_t number_of_known_objects() const;
		virtual size_t number_of_known_classes() const;

		// Helpers to pretty-print the entity tree for debugging
		virtual std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const = 0;
		friend std::ostream& operator<<(std::ostream& os, const Entity& node) {
			return node.prettyPrint(os, 0);
		}
};

} // namespace bpp::IR
