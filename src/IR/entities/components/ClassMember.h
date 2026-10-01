/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>

namespace bpp::IR::Components {

/**
 * @brief A base class for entities which are members of classes (methods and datamembers)
 * A class member has a visibility scope modifier (public, private, protected),
 * and is optionally marked as a "child" of a parent class's version.
 */
class ClassMember {
	private:
		VisibilityScope scope = VisibilityScope::PRIVATE;
		const ClassMember* parent_member = nullptr;

	public:
		void setScope(VisibilityScope scope) { this->scope = scope; }
		VisibilityScope getScope() const { return scope; }

	protected:
		void setParentMember(const ClassMember* parent_member) { this->parent_member = parent_member; }
		const ClassMember* getParentMember() const { return parent_member; }
};

} // namespace bpp::IR::Components
