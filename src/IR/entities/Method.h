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
		bool addParameter(std::shared_ptr<MethodParameter> parameter);
		const std::vector<std::shared_ptr<MethodParameter>>& getParameters() const { return parameters; }

		/**
		 * @brief Reserve space for a number of parameters in the parameters vector
		 *
		 * Used by the Listener to avoid repeated reallocations when adding parameters to a method.
		 * 
		 * @param count The number of parameters to reserve space for
		 */
		void reserveParameters(std::size_t count) { parameters.reserve(count); }

		std::string getAddress() const override;

		void setIsVirtual(bool is_virtual) { this->m_is_virtual = is_virtual; }
		bool isVirtual() const { return m_is_virtual; }

		void setIsOverridable(bool is_overridable) { this->m_is_overridable = is_overridable; }
		bool isOverridable() const { return m_is_overridable; }

		void setParentMethod(std::shared_ptr<Method> parent_method) { setParentMember(parent_method); }
		std::shared_ptr<Method> getParentMethod() const { return std::static_pointer_cast<Method>(getParentMember()); }

		void addReferencePosition(const SymbolPosition& pos) override;

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
