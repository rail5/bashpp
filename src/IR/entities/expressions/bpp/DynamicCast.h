/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>

namespace bpp::IR {

class DynamicCast : public StringType {
	private:
		/**
		 * @brief The type to which the dynamic cast is being performed.
		 * If the user gave the name of a class directly, this will be a RawCode containing the class name.
		 * Otherwise (if the user gave an expression which will expand to a class name at runtime),
		 * this will be a pointer to the entity whose code generation will produce that result.
		 */
		RawCodeOrOwnedEntity target_type;

	public:
		RawCodeOrUnownedEntity getTargetType() const;
		void setTargetType(const RawCode& type);
		void setTargetType(std::unique_ptr<Entity> type);

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state, const std::string& target_var) const;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
