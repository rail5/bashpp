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
		void setLvalueNonprimitive(bool is_nonprimitive) { lvalue_nonprimitive = is_nonprimitive; }
		void setRvalueNonprimitive(bool is_nonprimitive) { rvalue_nonprimitive = is_nonprimitive; }
		void setLvalueObject(std::shared_ptr<Object> obj) { lvalue_object = std::move(obj); }
		void setRvalueObject(std::shared_ptr<Object> obj) { rvalue_object = std::move(obj); }
		void setArrayAssignment(bool is_array_assignment) { array_assignment = is_array_assignment; }
		void setAdding(bool is_adding) { adding = is_adding; }

		bool isLvalueNonprimitive() const { return lvalue_nonprimitive; }
		bool isRvalueNonprimitive() const { return rvalue_nonprimitive; }
		std::shared_ptr<Object> getLvalueObject() const { return lvalue_object; }
		std::shared_ptr<Object> getRvalueObject() const { return rvalue_object; }
		bool isArrayAssignment() const { return array_assignment; }
		bool isAdding() const { return adding; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
