/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include "String.h"

namespace bpp::IR {

class DynamicCast : public String {
	private:
		std::shared_ptr<Class> target_type;

		// For codegen:
		/// If set, this variable will be used to store the result of the dynamic cast, instead of a temporary variable. This is used in cases like method parameters, where the result of the dynamic cast needs to be stored in a specific variable (i.e., the parameter name)
		std::optional<std::string> target_variable;
	public:
		DynamicCast() = delete;
		explicit DynamicCast(std::shared_ptr<Class> target_type) : target_type(target_type) {}
		std::shared_ptr<Class> get_target_type() const { return target_type; }

		void set_target_variable(const std::string& var_name) { target_variable = var_name; }

		/// Get the name of the variable to which the result of this dynamic cast should be assigned in the generated code. This will be either the explicitly set target variable, or a generated temporary variable if no target variable was explicitly set.
		std::string get_target_variable() const;

		bpp::CodeGen::CodeSegment generate_code() override;

		PRETTYPRINT_OVERRIDE;
};

} // namespace bpp::IR
