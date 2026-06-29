/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include "String.h"

namespace bpp::IR {

class DynamicCast : public StringType {
	private:
		/**
		 * @brief The type to which the dynamic cast is being performed.
		 * If the user gave the name of a class directly, this will be a RawCode containing the class name.
		 * Otherwise (if the user gave an expression which will expand to a class name at runtime),
		 * this will be a pointer to the entity whose code generation will produce that result.
		 */
		RawCodeOrEntity target_type;

		// For codegen:
		/// If set, this variable will be used to store the result of the dynamic cast, instead of a temporary variable. This is used in cases like method parameters, where the result of the dynamic cast needs to be stored in a specific variable (i.e., the parameter name)
		std::optional<std::string> target_variable;
	public:
		RawCodeOrEntity get_target_type() const { return target_type; }
		void set_target_type(const RawCodeOrEntity& type) { target_type = type; }

		void set_target_variable(const std::string& var_name) { target_variable = var_name; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
