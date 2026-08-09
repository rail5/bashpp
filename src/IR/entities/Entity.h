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
#include <IR/codegen.h>

#include <debug_helpers.h>

namespace bpp::IR {

/**
 * @brief The base class for all entities in Bash++
 *
 * All constructs in Bash++ are entities. This includes classes, methods, objects, and even the program itself.
 * It also includes constructs such as compound statements (e.g., 'if' statements and 'while' loops), although these entities don't have names.
 */
class Entity {
	protected:
		std::size_t parent_visible_object_count_at_creation = 0;
		std::size_t program_visible_class_count_at_creation = 0;

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

		/// A list of all entities that reference this entity (used for optimization and dead-code elimination)
		std::list<std::weak_ptr<Entity>> referencing_entities;
	public:
		Entity() = default;
		virtual ~Entity() = default;

		Entity(const Entity& other) = default;
		Entity& operator=(const Entity& other) = default;
		Entity(Entity&& other) = default;
		Entity& operator=(Entity&& other) = default;

		virtual std::weak_ptr<Class> get_containing_class() { return containing_class; }
		virtual std::weak_ptr<const Class> get_containing_class_const() const { return containing_class; }
		void set_containing_class(std::weak_ptr<Class> containing_class) { this->containing_class = std::move(containing_class); }

		virtual std::weak_ptr<Program> get_containing_program() { return containing_program; }
		virtual std::weak_ptr<const Program> get_containing_program_const() const { return containing_program; }
		void set_containing_program(std::weak_ptr<Program> containing_program) { this->containing_program = std::move(containing_program); }

		SymbolPosition get_definition_position() const { return definition_position; }
		void set_definition_position(const SymbolPosition& pos) { this->definition_position = pos; }

		const std::list<SymbolPosition>& get_reference_positions() const { return reference_positions; }

		// Note: Methods require a different procedure.
		// Adding a reference to a derived class's version of an inherited method should also add a reference
		// to the base class's version of the method, since both are considered "used" in that case.
		// Likewise for data members.
		virtual void add_reference_position(const SymbolPosition& pos) { this->reference_positions.push_back(pos); }

		/**
		 * @brief Inherit from another entity
		 *
		 * Inheritance means:
		 *
		 * - The parent entity is marked as a "parent" of this entity
		 *
		 * - This entity inherits the parent's "containing program" and "containing class" if it doesn't already have them
		 *
		 * - This entity inherits the parent's map of known objects and classes, but only up to the point where the parent entity was created.
		 *   This means that if the parent entity creates new objects or classes after this entity is created,
		 *   those new objects and classes will not be visible to this entity.
		 * 
		 * @param parent The entity to inherit from
		 */
		void inherit(std::shared_ptr<Entity> parent);

		/**
		 * @brief Get a class by name
		 *
		 * All classes are owned by the program (root node of the entity tree), not by inner entities.
		 *
		 * This entity will only be able to see classes that existed at the time of its creation, and not any classes that were created later.
		 * 
		 * @param name The name of the class to get
		 * @param max_visible_index The maximum visible index of the class to get (for scoping purposes)
		 * @return std::shared_ptr<Class> The class, or nullptr if not found
		 */
		virtual std::shared_ptr<Class> get_class(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const;

		/**
		 * @brief Get an object by name
		 *
		 * This entity will only be able to see:
		 *
		 * - Objects that existed at the time of its creation
		 *
		 * - Objects that are owned by this entity directly (i.e., this entity is responsible for their lifetime)
		 * 
		 * @param name The name of the object to get
		 * @param max_visible_index The maximum visible index of the object to get (for scoping purposes)
		 * @return std::shared_ptr<Object> The object, or nullptr if not found
		 */
		virtual std::shared_ptr<Object> get_object(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const;

		virtual std::vector<std::shared_ptr<Class>> get_all_known_classes() const;
		virtual std::vector<std::shared_ptr<Object>> get_all_known_objects() const;

		virtual std::size_t number_of_known_objects() const;
		virtual std::size_t number_of_known_classes() const;

		virtual bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* /*state*/) const { return {}; }

		bool is_referenced() const;
		void mark_referenced_by(std::shared_ptr<Entity> referencing_entity) { referencing_entities.push_back(referencing_entity); }
		const std::list<std::weak_ptr<Entity>>& get_referencing_entities() const { return referencing_entities; }

		PRETTYPRINT_HELPERS(Entity)
};

} // namespace bpp::IR
