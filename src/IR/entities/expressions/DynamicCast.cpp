/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DynamicCast.h"

#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment DynamicCast::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "DynamicCast::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;

	const auto& inner_code = StringType::generate_code(state);

	result.add_pre_code(inner_code.get_pre_code());
	result.add_post_code(inner_code.get_post_code());

	const std::string reference_value = [&inner_code]() {
		std::string result;
		for (const auto& part : inner_code.get_main_code()) result += part;
		return result;
	}();

	// The 'main code' of the inner expression is the reference value, i.e., the address of the object to be casted.

	const std::string result_variable = [state, this]() {
		if (this->target_variable.has_value()) return this->target_variable.value();
		return "__dynamicCast" + std::to_string(state->dynamic_cast_counter++);
	}();
	
	// If no target variable was explicitly set, this dynamic cast is being used as a temporary value
	// so we should unset the result variable after using it to avoid cluttering the generated code with unnecessary variables.
	if (!target_variable.has_value()) result.add_post_code("\nunset " + result_variable + "\n");

	bpp::CodeGen::CodeSegment cast_to;
	if (std::holds_alternative<RawCode>(target_type)) {
		cast_to.add_main_code(std::get<RawCode>(target_type));
	} else if (std::holds_alternative<std::shared_ptr<Entity>>(target_type)) {
		auto entity = std::get<std::shared_ptr<Entity>>(target_type);
		cast_to.egalitarian_merge(entity->generate_code(state));
	}

	result.add_pre_code(cast_to.get_pre_code());
	result.add_post_code(cast_to.get_post_code());
	const std::string cast_to_value = [&cast_to]() {
		std::string result;
		for (const auto& part : cast_to.get_main_code()) result += part;
		return result;
	}();
	
	result.add_pre_code("bpp____dynamic_cast \"" + cast_to_value + "\""
		+ " \"" + result_variable + "\""
		+ " \"" + reference_value + "\"\n");

	result.add_main_code("${" + result_variable + "}");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(DynamicCast, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(DynamicCast\n"
		<< indent << "TargetType:\n";
	if (std::holds_alternative<RawCode>(target_type)) {
		os << indent << std::get<RawCode>(target_type) << "\n";
	} else if (std::holds_alternative<std::shared_ptr<Entity>>(target_type)) {
		auto entity = std::get<std::shared_ptr<Entity>>(target_type);
		entity->prettyPrint(os, indentation_level + 1);
	} else {
		os << indent << "<Invalid target type>\n";
	}
	os << indent << "Input:\n";
	StringType::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
