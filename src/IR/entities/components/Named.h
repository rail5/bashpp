/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <string_view>

#include <IR/entities/components/Referenceable.h>

namespace bpp::IR::Components {

/**
 * @brief A base class for entities which have names.
 *
 * Not all entities have names, but those that do (e.g., classes, methods, objects) inherit from this class *as well as* from Entity.
 *
 * Any entity which is named is also referenceable.
 */
class Named : public Referenceable {
	protected:
		std::string name;

		explicit Named(const std::string& name) : name(name) {}
		~Named() = default;
		Named(const Named& other) = default;
		Named(Named&& other) noexcept = default;
		Named& operator=(const Named& other) = default;
		Named& operator=(Named&& other) noexcept = default;

	public:
		Named() = delete;

		const std::string& getName() const { return name; }
		std::string_view viewName() const { return name; }

		void setDefinitionPosition(const SourceLocation& position) {
			Referenceable::setDefinitionPosition({
				position.source_file,
				position.position,
				{position.position.line, static_cast<std::uint32_t>(position.position.column + name.length())}
			});
		}

		virtual void addReferencePosition(const SourceLocation& position) const {
			Referenceable::addReferencePosition({
				position.source_file,
				position.position,
				{position.position.line, static_cast<std::uint32_t>(position.position.column + name.length())}
			});
		}
};

} // namespace bpp::IR::Components
