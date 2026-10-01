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

RawCodeOrUnownedEntity DynamicCast::getTargetType() const {
	if (std::holds_alternative<RawCode>(target_type)) {
		return std::get<RawCode>(target_type);
	} else if (std::holds_alternative<std::unique_ptr<Entity>>(target_type)) {
		return std::get<std::unique_ptr<Entity>>(target_type).get();
	}

	throw bpp::ErrorHandling::InternalError("DynamicCast::getTargetType(): target_type is in an invalid state");
}

void DynamicCast::setTargetType(const RawCode& type) {
	target_type = type;
}

void DynamicCast::setTargetType(std::unique_ptr<Entity> type) {
	target_type = std::move(type);
}

bpp::CodeGen::CodeSegment DynamicCast::generateCode(bpp::CodeGen::CodeGenState* state) const {
	return generateCode(state, "");
}

bpp::CodeGen::CodeSegment DynamicCast::generateCode(bpp::CodeGen::CodeGenState* state, const std::string& target_var) const {
	bpp_assert(state != nullptr, "State pointer is null");
	state->requires_dynamic_cast_function = true;
	bpp::CodeGen::CodeSegment result;

	const auto& inner_code = StringType::generateCode(state);

	result.add_pre_code(inner_code.get_pre_code());
	result.add_post_code(inner_code.get_post_code());

	const std::string reference_value = [&inner_code]() {
		std::string result;
		for (const auto& part : inner_code.get_main_code()) result += part;
		return result;
	}();

	// The 'main code' of the inner expression is the reference value, i.e., the address of the object to be casted.

	const std::string result_variable = [state, target_var]() {
		if (!target_var.empty()) return target_var;
		return "__dynamicCast" + std::to_string(state->dynamic_cast_counter++);
	}();

	// If no target variable was explicitly set, this dynamic cast is being used as a temporary value
	// so we should unset the result variable after using it to avoid cluttering the generated code with unnecessary variables.
	if (target_var.empty()) result.add_post_code("\nunset " + result_variable + "\n");

	bpp::CodeGen::CodeSegment cast_to;
	if (std::holds_alternative<RawCode>(target_type)) {
		cast_to.copy_to_main_code(std::get<RawCode>(target_type));
	} else if (std::holds_alternative<std::unique_ptr<Entity>>(target_type)) {
		const auto* entity = std::get<std::unique_ptr<Entity>>(target_type).get();
		cast_to.egalitarian_merge(entity->generateCode(state));
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
		<< indent << "TargetType: ";
	if (std::holds_alternative<RawCode>(target_type)) {
		os << std::get<RawCode>(target_type) << "\n";
	} else if (std::holds_alternative<std::unique_ptr<Entity>>(target_type)) {
		os << "\n";
		const auto* entity = std::get<std::unique_ptr<Entity>>(target_type).get();
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
