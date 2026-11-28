/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/components/Named.h>
#include <IR/entities/components/Addressable.h>

namespace bpp::IR {

/**
 * @brief A normal shell function
 *
 * Functions are both CodeEntities (which makes them executable) and Addressable entities (can be addressed).
 * This combination makes them "callable."
 *
 * A callable entity cannot inherit object visibility from nested parents,
 * because it is not guaranteed that the objects owned by those nested parents
 * will have been instantiated before the callable entity is executed,
 * since it can be called *directly* from anywhere in the program, not only from the scope in which it was defined.
 *  (E.g., an outer function + an inner function: the inner function cannot assume that
 *   the outer function's objects will exist at the time of its execution.)
 * Callable entities therefore only inherit object visibility from the Program.
 * If you want access to objects owned by a parent entity, you must pass them as arguments to the callable entity.
 * In the case of Methods, this is done automatically by the implicit `this` pointer argument.
 */
class BashFunction : public CodeEntity, public Components::Named, public Components::Addressable {
	public:
		BashFunction() = delete;
		explicit BashFunction(const std::string& name) : Components::Named(name) {}
		std::string getAddress() const override { return getName(); }
		void inherit(const Entity* parent) override;
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
