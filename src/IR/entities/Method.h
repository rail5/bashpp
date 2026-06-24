/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/BashFunction.h>
#include <IR/entities/Object.h>

#include <vector>
#include <memory>

namespace bpp::IR {

/**
 * @brief A parameter to a method in a class
 *
 * Note that some parameters may be removed from the method's parameter list by optimizations, if they are unused.
 *
 * For this reason it's important to retain the index of the parameter in the original parameter list,
 * so that it can be matched up with the corresponding argument in the method call (e.g. `local arg3="$3"`),
 * rather than just relying on the parameter's position in the parameter list.
 */
class MethodParameter : public Object {
	protected:
		/// The index of this parameter in the method's parameter list (1-based)
		uint32_t index = 1;
		std::weak_ptr<Method> containing_method;
	public:
		MethodParameter() = delete;
		explicit MethodParameter(std::shared_ptr<Method> method) : containing_method(method) {}
		uint32_t get_index() const { return index; }
		void set_index(uint32_t index) { this->index = index; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

/**
 * @brief The implicit `this` parameter of a method, which refers to the object on which the method was called.
 */
class ThisPtr : public MethodParameter {
	public:
		ThisPtr() = delete;
		explicit ThisPtr(std::shared_ptr<Method> containing_method);

		std::string get_address() const override { return "__this"; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

/**
 * @brief A method in a class
 */
class Method : public BashFunction {
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
		bool add_parameter(std::shared_ptr<MethodParameter> parameter);
		const std::vector<std::shared_ptr<MethodParameter>>& get_parameters() const { return parameters; }

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

		std::string get_mangled_name() const;
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
