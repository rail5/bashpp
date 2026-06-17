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
	public:
		void set_scope(VisibilityScope scope) { this->scope = scope; }
		VisibilityScope get_scope() const { return scope; }

		void set_is_array(bool is_array) { this->m_is_array = is_array; }
		bool is_array() const { return m_is_array; }
};

} // namespace bpp::IR
