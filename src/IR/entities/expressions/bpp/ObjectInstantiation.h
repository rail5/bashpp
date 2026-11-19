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
 *
 * __new can be called in two ways:
 *  1. Heap-like: `@new TYPE`:
 *     In this case,
 *      - The object's address is determined at runtime by __new
 *      - __new outputs this address, which is substituted in place of the `@new` expression.
 *      - The containing scope will not automatically manage the object's lifetime
 *  2. Stack-like: `@TYPE ID`:
 *     In this case,
 *      - The object's address is known at compile-time
 *      - The call to __new explicitly requests this address (s.t. __new acts as "placement new")
 *      - The containing scope will automatically manage the object's lifetime
 *
 * An object's constructor is called immediately after __new, if it exists.
 * Parent-class constructors are called first, in order of inheritance
 * (i.e., the constructor of the most distant ancestor is called first, and the constructor of the most derived class is called last).
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
