/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/BashFunction.h>
#include <IR/entities/Object.h>
#include <IR/entities/AddressableEntity.h>
#include <IR/entities/MethodParameter.h>

#include <vector>
#include <memory>

namespace bpp::IR {

/**
 * @brief A method in a class
 */
class Method : public BashFunction, public AddressableEntity, public std::enable_shared_from_this<Method> {
	private:
		/// List of parameters expected to be given as arguments to the method
		std::vector<std::shared_ptr<MethodParameter>> parameters;
		VisibilityScope scope = VisibilityScope::PUBLIC;

		bool m_is_virtual = false;
		bool m_is_overridable = false;
		bool m_is_inherited = false;

		/// If this method is inherited from a parent class (or even overridden), this points to the parent class's version of this method.
		std::weak_ptr<Method> parent_method;
	public:
		/**
		 * @brief Add a parameter to this method
		 *
		 * If this parameter's name conflicts with an existing parameter, this method will return false and not add the parameter.
		 *
		 * If the parameter is a pointer to a nonprimitive type, this method will also set up an implicit dynamic cast for the parameter,
		 * and verify that its name does not conflict with any known classes or objects in the containing method's context.
		 * 
		 * @param parameter The parameter to add
		 * @return true if the parameter was added successfully, false otherwise
		 */
		bool add_parameter(std::shared_ptr<MethodParameter> parameter);
		const std::vector<std::shared_ptr<MethodParameter>>& get_parameters() const { return parameters; }

		std::string get_address() const override;

		void set_scope(VisibilityScope scope) { this->scope = scope; }
		VisibilityScope get_scope() const { return scope; }

		void set_is_virtual(bool is_virtual) { this->m_is_virtual = is_virtual; }
		bool is_virtual() const { return m_is_virtual; }

		void set_is_overridable(bool is_overridable) { this->m_is_overridable = is_overridable; }
		bool is_overridable() const { return m_is_overridable; }

		void set_is_inherited(bool is_inherited) { this->m_is_inherited = is_inherited; }
		bool is_inherited() const { return m_is_inherited; }

		void set_parent_method(std::shared_ptr<Method> parent_method) { this->parent_method = parent_method; }
		std::shared_ptr<Method> get_parent_method() const { return parent_method.lock(); }

		void add_reference_position(const SymbolPosition& pos) override;

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
