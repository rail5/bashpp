/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <memory>

namespace bpp::IR {

/**
 * @brief A base class for entities which are members of classes (methods and datamembers)
 * A class member has a visibility scope modifier (public, private, protected),
 * and is optionally marked as a "child" of a parent class's version.
 */
class ClassMemberEntity {
	private:
		VisibilityScope scope = VisibilityScope::PRIVATE;
		std::weak_ptr<ClassMemberEntity> parent_member;

	public:
		void setScope(VisibilityScope scope) { this->scope = scope; }
		VisibilityScope getScope() const { return scope; }

	protected:
		void setParentMember(std::shared_ptr<ClassMemberEntity> parent_member) { this->parent_member = parent_member; }
		std::shared_ptr<ClassMemberEntity> getParentMember() const { return parent_member.lock(); }
};

} // namespace bpp::IR
