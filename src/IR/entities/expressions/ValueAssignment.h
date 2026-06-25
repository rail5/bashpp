/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>

namespace bpp::IR {

class ValueAssignment : public StringType {
	private:
		bool lvalue_nonprimitive = false;
		bool rvalue_nonprimitive = false;
		std::shared_ptr<Object> lvalue_object = nullptr;
		std::shared_ptr<Object> rvalue_object = nullptr;

		bool array_assignment = false; // E.g arr=()
		bool adding = false; // E.g. arr+=("value")
	public:
		void set_rvalue_nonprimitive(bool is_nonprimitive) { rvalue_nonprimitive = is_nonprimitive; }
		void set_lvalue_nonprimitive(bool is_nonprimitive) { lvalue_nonprimitive = is_nonprimitive; }
		void set_lvalue_object(std::shared_ptr<Object> obj) { lvalue_object = obj; }
		void set_rvalue_object(std::shared_ptr<Object> obj) { rvalue_object = obj; }
		void set_array_assignment(bool is_array_assignment) { array_assignment = is_array_assignment; }
		void set_adding(bool is_adding) { adding = is_adding; }

		bool is_lvalue_nonprimitive() const { return lvalue_nonprimitive; }
		bool is_rvalue_nonprimitive() const { return rvalue_nonprimitive; }
		std::shared_ptr<Object> get_lvalue_object() const { return lvalue_object; }
		std::shared_ptr<Object> get_rvalue_object() const { return rvalue_object; }
		bool is_array_assignment() const { return array_assignment; }
		bool is_adding() const { return adding; }
};

} // namespace bpp::IR
