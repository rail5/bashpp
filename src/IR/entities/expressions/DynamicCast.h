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
	public:
		DynamicCast() = delete;
		explicit DynamicCast(std::shared_ptr<Class> target_type) : target_type(target_type) {}
		std::shared_ptr<Class> get_target_type() const { return target_type; }

		bpp::CodeGen::CodeSegment generate_code() override;

		PRETTYPRINT_OVERRIDE;
};

} // namespace bpp::IR
