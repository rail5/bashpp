/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/expressions/ValueAssignment.h>

namespace bpp::IR {

/**
 * @brief A declaration of a pointer, which may optionally include an initial value, and is assigned @nullptr (0) if no initial value is provided.
 */
class PointerDeclaration : public Entity {
	private:
		const Object* pointer = nullptr;
		std::unique_ptr<ValueAssignment> initial_value;
	public:
		const Object* getPointer() const { return pointer; }
		void setPointer(const Object* p) { pointer = p; }

		const ValueAssignment* getInitialValue() const { return initial_value.get(); }
		void setInitialValue(std::unique_ptr<ValueAssignment> iv) { initial_value = std::move(iv); }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
