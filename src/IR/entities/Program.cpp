/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Program.h"
#include "Class.h"

#include <error/InternalError.h>

namespace bpp::IR {

bool Program::add_class(std::shared_ptr<Class> class_entity) {
	if (classes.find(class_entity->get_name())) return false; // Class with this name already exists

	// Add the class to 2 places:
	
	// 1. To our global list of known classes, so that it can be found by name later
	classes.add(class_entity);

	// 2. To the entity tree, so that it can be traversed later
	//    (e.g., its position in the entity tree signifies where its definition should be placed)
	this->add(class_entity);

	return true;
}

bpp::CodeGen::CodeSegment Program::generate_code() {
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("#!/usr/bin/env bash\n");
	code.egalitarian_merge(CodeEntity::generate_code());

	return code;
}

} // namespace bpp::IR
