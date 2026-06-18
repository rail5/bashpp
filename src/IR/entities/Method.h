/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/BashFunction.h>

#include <vector>
#include <memory>

namespace bpp::IR {

/**
 * @brief A method in a class
 */
class Method : public BashFunction {
	private:
		/// List of parameters expected to be given as arguments to the method
		std::vector<std::shared_ptr<Object>> parameters;
		VisibilityScope scope = VisibilityScope::PUBLIC;

		bool m_is_virtual = false;
		bool m_is_overridable = false;
		bool m_is_inherited = false;
	public:
		bool add_parameter(std::shared_ptr<Object> parameter);
		const std::vector<std::shared_ptr<Object>>& get_parameters() const { return parameters; }

		void set_scope(VisibilityScope scope) { this->scope = scope; }
		VisibilityScope get_scope() const { return scope; }

		void set_is_virtual(bool is_virtual) { this->m_is_virtual = is_virtual; }
		bool is_virtual() const { return m_is_virtual; }

		void set_is_overridable(bool is_overridable) { this->m_is_overridable = is_overridable; }
		bool is_overridable() const { return m_is_overridable; }

		void set_is_inherited(bool is_inherited) { this->m_is_inherited = is_inherited; }
		bool is_inherited() const { return m_is_inherited; }

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override;
};

} // namespace bpp::IR
