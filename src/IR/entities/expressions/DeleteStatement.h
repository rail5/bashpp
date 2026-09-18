/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/expressions/ObjectReference.h>

namespace bpp::IR {

class DeleteStatement : public CodeEntity, public std::enable_shared_from_this<DeleteStatement> {
	private:
		std::shared_ptr<ObjectReference> destructor_method_call;
		std::shared_ptr<ObjectReference> delete_method_call;
	public:
		void setObjectToDelete(std::shared_ptr<ObjectReference> obj_ref);

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
