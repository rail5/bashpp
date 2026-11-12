/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "IncludeStatement.h"

#include <IR/entities/Program.h>
#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment IncludeStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");

	if (!codegen_enabled) return {};

	if (isDynamicInclude()) {
		// Dynamic include: skip codegen for child entities,
		// just place a `source` command for the compiled version of the included file.
		bpp::CodeGen::CodeSegment result;
		const auto path = getRuntimePath().string();
		result.add_main_code(
			"if ! source \"" + path + "\"; then\n"
			"\t>&2 echo \"Bash++: Error: Failed to include file '" + path + "'\"\n"
			"\tbpp____destroy_objectStack\n"
			"\texit 1\n"
			"fi\n"
		);
		return result;
	} else {
		// Static include: effectively inline the included file's code
		return CodeEntity::generateCode(state);
	}
}

std::filesystem::path IncludeStatement::getRuntimePath() const {
	if (as_path.has_value()) return as_path.value();

	auto runtime_path = included_file_path;
	return runtime_path.replace_extension(".sh");
}

} // namespace bpp::IR
