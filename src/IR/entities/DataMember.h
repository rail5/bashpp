/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/Object.h>

namespace bpp::IR {

/**
 * @brief A data member in a class
 *
 * Although this inherits from Object, it can also be a primitive.
 * The case in which the data member is a primitive is represented by type == nullptr.
 */
class DataMember : public Object {
	private:
		VisibilityScope scope = VisibilityScope::PRIVATE;
		bool m_is_array = false;

		/// If this data member is inherited from a parent class, this points to the parent class's version of this data member.
		std::weak_ptr<DataMember> parent_datamember;
	public:
		void set_scope(VisibilityScope scope) { this->scope = scope; }
		VisibilityScope get_scope() const { return scope; }

		void set_is_array(bool is_array) { this->m_is_array = is_array; }
		bool is_array() const { return m_is_array; }

		void set_parent_datamember(std::shared_ptr<DataMember> parent_datamember) { this->parent_datamember = parent_datamember; }
		std::shared_ptr<DataMember> get_parent_datamember() const { return parent_datamember.lock(); }

		void add_reference_position(const SymbolPosition& pos) override;
};

} // namespace bpp::IR
