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
	private:
		/// The children of this node in the entity tree
		std::vector<RawCodeOrEntity> children;
	public:
		const std::vector<RawCodeOrEntity>& get_children() const { return children; }
		void add(const RawCode& child);
		void add(const std::shared_ptr<Entity>& child);

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override;
};

} // namespace bpp::IR
