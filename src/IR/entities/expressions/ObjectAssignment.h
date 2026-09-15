/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>
#include <IR/entities/expressions/ObjectReference.h>
#include <IR/entities/expressions/ValueAssignment.h>

namespace bpp::IR {

/**
 * @brief An assignment statement where the left-hand side is an object reference
 *
 * E.g., @object.dataMember="value"
 * Or @obj1=@obj2
 * Etc
 */
class ObjectAssignment : public StringType, public std::enable_shared_from_this<ObjectAssignment> {
	private:
		std::shared_ptr<ObjectReference> lhs;
		std::shared_ptr<ValueAssignment> rhs;
	public:
		void setLHS(std::shared_ptr<ObjectReference> l) {
			lhs = std::move(l);
			lhs->setLvalue(true);
			lhs->setAddressOf(true);
		}
		void setRHS(std::shared_ptr<ValueAssignment> r) { rhs = std::move(r); }

		std::shared_ptr<const ObjectReference> getLHS() const { return lhs; }
		std::shared_ptr<const ValueAssignment> getRHS() const { return rhs; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
