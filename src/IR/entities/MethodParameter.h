/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Object.h>

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
		std::uint32_t index = 1;
	public:
		std::uint32_t get_index() const { return index; }
		void set_index(std::uint32_t index) { this->index = index; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

/**
 * @brief The implicit `this` parameter of a method, which refers to the object on which the method was called.
 */
class ThisPtr : public MethodParameter {
	public:
		ThisPtr() = delete;
		explicit ThisPtr(std::shared_ptr<const Class> containing_class) {
			set_name("this");
			set_type(containing_class);
			set_is_pointer(true);
		}

		std::string get_address() const override { return "__this"; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

/**
 * @brief A special parameter passed to __new, which is the optional requested address of the new object to be created.
 * If the caller does not specify a requested address, this parameter will be empty, and the __new method will generate a new address for the object.
 */
class RequestedAddressParam : public ThisPtr {
	public:
		RequestedAddressParam() = delete;
		explicit RequestedAddressParam(std::shared_ptr<const Class> containing_class) : ThisPtr(containing_class) {}

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

} // namespace bpp::IR
