/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <vector>
#include <variant>

#include <IR/bpp.h>
#include <IR/entities/Entity.h>

namespace bpp::IR {

using RawCode = std::string;
using RawCodeOrEntity = std::variant<RawCode, std::shared_ptr<Entity>>;

/**
 * @brief Any entity which can contain executable code.
 *
 * This includes the Program, all Methods, etc.
 * 
 * Notably, classes and objects cannot contain executable code, and are not therefore code entities.
 */
class CodeEntity : public Entity {
	protected:
		/// Objects owned by this entity (i.e., this entity is responsible for their lifetime)
		OwnedEntityList<Object> local_objects;
		/// The children of this node in the entity tree
		std::vector<RawCodeOrEntity> children;
	public:
		const std::vector<RawCodeOrEntity>& get_children() const { return children; }

		/**
		 * @brief Add raw code to the entity tree as a child of this code entity
		 * 
		 * @param child The raw code to add
		 */
		void add(const RawCode& child);

		/**
		 * @brief Add another entity to the entity tree as a child of this code entity
		 * 
		 * @param child The entity to add
		 */
		void add(const std::shared_ptr<Entity>& child);

		/**
		 * @brief Add an object to this code entity, making it responsible for the object's lifetime.
		 *
		 * If the object name conflicts with an existing object or class known to this code entity, this function will return false and not add the object.
		 *
		 * Note that this does not guarantee instantiation.
		 * The Listener (or other caller) is responsible for ensuring that the Method call necessary
		 * to instantiate the object is added to the entity tree at the appropriate location.
		 * 
		 * @param object The object to add
		 */
		bool add_object(std::shared_ptr<Object> object);
		const OwnedEntityList<Object>& get_local_objects() const { return local_objects; }

		/**
		 * @brief Get an object by name, searching local objects first, then parent entities
		 * 
		 * @param name The name of the object to get
		 * @param max_visible_index The maximum visible index of the object to get (for scoping purposes)
		 * @return std::shared_ptr<Object> The object, or nullptr if not found
		 */
		std::shared_ptr<Object> get_object(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const override;

		/**
		 * @brief Get a list of all objects known to this code entity, whether owned by this code entity or merely visible to it
		 * 
		 * @return std::vector<std::shared_ptr<Object>> A vector of all objects known to this code entity
		 */
		std::vector<std::shared_ptr<Object>> get_all_known_objects() const override;
		std::size_t number_of_known_objects() const override;

		/**
		 * @brief Adopt all local objects from another CodeEntity into this one.
		 *
		 * This code entity becomes responsible for the lifetimes (& ownership) of the adopted objects.
		 * 
		 * @param other The other CodeEntity from which to adopt local objects
		 */
		void adopt_objects_of(std::shared_ptr<CodeEntity> other);

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
