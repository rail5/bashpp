/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>

#include <IR/bpp.h>
#include <IR/entities/expressions/bpp/ObjectReference.h>

#include "Listener.h"

namespace bpp::AST {

class Listener::EntityFabricator {
	public:
		static void makeNonprimitiveCopy(
			Listener* listener,
			ASTNode* node,
			std::unique_ptr<bpp::IR::ObjectReference> lhs,
			std::unique_ptr<bpp::IR::ObjectReference> rhs
		);
};

} // namespace bpp::AST
