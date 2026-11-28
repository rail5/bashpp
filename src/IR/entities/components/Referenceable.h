/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <deque>
#include <optional>

#include <IR/SourceLocation.h>
#include <include/EntityMap.h>
#include <IR/entities/Entity.h>
#include <error/InternalError.h>

namespace bpp::IR::Components {

/**
 * @brief A base class for entities which can be referenced in source code.
 *
 * Generally, this means that the entity has a name.
 * Rarely, though, it is possible for an entity to be referenceable without having a name (e.g., an included file).
 */
class Referenceable {
	private:
		std::optional<SourceLocationRange> definition_position;

		/**
		 * @brief A list of all positions in source files where this entity is referenced.
		 *
		 * This is logically const, in the sense that it does not affect the public API of the entity,
		 * but it is mutable because it is updated as the source files are processed.
		 *
		 * The const-ness of this should be re-evaluated if reference positions are ever allowed
		 * to be modified or removed. As it stands, they can only be appended.
		 */
		mutable std::deque<SourceLocationRange> reference_positions;
	public:
		const std::optional<SourceLocationRange>& getDefinitionPosition() const { return definition_position; }
		const std::deque<SourceLocationRange>& getReferencePositions() const { return reference_positions; }
	protected:
		Referenceable() = default;
		void setDefinitionPosition(const SourceLocationRange& position) { definition_position = position; }
		void addReferencePosition(const SourceLocationRange& position) const { reference_positions.push_back(position); }
};

} // namespace bpp::IR::Components
