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
		void add(const RawCode& child);
		void add(const std::shared_ptr<Entity>& child);

		bool add_object(std::shared_ptr<Object> object);
		const OwnedEntityList<Object>& get_local_objects() const { return local_objects; }

		std::shared_ptr<Object> get_object(const std::string& name, size_t max_visible_index = SIZE_MAX) override;
		std::vector<std::shared_ptr<Object>> get_all_known_objects() const override;
		size_t number_of_known_objects() const override;

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
