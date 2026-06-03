/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <list>

#include "bpp.h"

namespace bpp {

/**
 * @class bpp_entity
 * @brief The base class for all entities in the Bash++ compiler
 * 
 * An entity is a class, object, method, or other construct in the Bash++ compiler.
 * This class provides the basic functionality for all entities.
 */
class bpp_entity {
	protected:
		std::string name;
		
		OwnedEntityList<bpp_object> local_objects; // Objects owned by this entity
		size_t parent_visible_object_count_at_creation = 0;
		size_t program_visible_class_count_at_creation = 0;

		std::weak_ptr<bpp_class> type;
		std::weak_ptr<bpp_class> containing_class;
		std::weak_ptr<bpp_program> containing_program;

		/// For classes: the list of parent classes, in order
		std::vector<std::weak_ptr<bpp_class>> parents;

		/// For all entities (except program), the parent entity from which this entity inherits
		std::weak_ptr<bpp_entity> parent_entity;
		
		std::weak_ptr<bpp_method> overridden_method;
		bpp::SymbolPosition initial_definition;
		std::list<bpp::SymbolPosition> references;
	public:
		bpp_entity() = default;
		virtual ~bpp_entity() = default;

		bpp_entity(const bpp_entity& other) = default;
		bpp_entity& operator=(const bpp_entity& other) = default;
		bpp_entity(bpp_entity&& other) noexcept = default;
		bpp_entity& operator=(bpp_entity&& other) noexcept = default;

		virtual bool add_object(std::shared_ptr<bpp_object> object, bool make_local = false);

		virtual std::shared_ptr<bpp_class> get_class();
		virtual std::string get_address() const;
		virtual void set_name(const std::string& name);
		virtual std::string get_name() const;
		virtual std::weak_ptr<bpp::bpp_class> get_containing_class();
		virtual std::weak_ptr<bpp_program> get_containing_program();
		virtual bool set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class);

		void inherit(std::shared_ptr<bpp_entity> parent);
		void inherit(std::shared_ptr<bpp_program> program);
		virtual void inherit(std::shared_ptr<bpp_class> parent);

		void set_definition_position(const std::string& file, uint64_t line, uint64_t column);
		void add_reference(const std::string& file, uint64_t line, uint64_t column);

		bpp::SymbolPosition get_initial_definition() const;
		std::list<bpp::SymbolPosition> get_references() const;

		virtual std::shared_ptr<bpp_class> get_class(const std::string& name, size_t max_visible_index = SIZE_MAX);
		std::shared_ptr<bpp_object> get_object(const std::string& name, size_t max_visible_index = SIZE_MAX);

		virtual std::vector<std::shared_ptr<bpp_class>> get_all_known_classes() const;
		virtual std::vector<std::shared_ptr<bpp_object>> get_all_known_objects() const;

		const OwnedEntityList<bpp_object>& get_local_objects() const;

		std::shared_ptr<bpp_class> get_parent() const;

		size_t number_of_known_objects() const;
		virtual size_t number_of_known_classes() const;
};

} // namespace bpp
