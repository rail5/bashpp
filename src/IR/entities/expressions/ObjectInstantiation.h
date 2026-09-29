/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>

namespace bpp::IR {

/**
 * @brief An instantiation of an object (i.e., a call to __new and optionally __constructor)
 */
class ObjectInstantiation : public Entity {
	private:
		const Class* type;
		const Object* stackLikeObject;

		bpp::CodeGen::CodeSegment heapLikeInstantiation(bpp::CodeGen::CodeGenState* state) const;
		bpp::CodeGen::CodeSegment stackLikeInstantiation(bpp::CodeGen::CodeGenState* state) const;
	public:
		const Class* getType() const { return type; }
		void setType(const Class* t) { type = t; }
		const Object* getStackLikeObject() const { return stackLikeObject; }
		void setStackLikeObject(const Object* o) { stackLikeObject = o; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
