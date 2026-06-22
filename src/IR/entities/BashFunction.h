/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>

namespace bpp::IR {

/**
 * @brief A normal shell function
 */
class BashFunction : public CodeEntity {
	protected:
		std::string name;
	public:
		const std::string& get_name() const { return name; }
		void set_name(const std::string& name) { this->name = name; }

		bpp::CodeGen::CodeSegment generate_code() override;

		PRETTYPRINT_OVERRIDE;
};

} // namespace bpp::IR
