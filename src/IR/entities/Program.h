/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <cstdint>

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>

namespace bpp::IR {

class Program : public CodeEntity {
	public:
		void add_diagnostic(std::string, bpp::diagnostic_type, std::string, uint32_t, uint32_t, uint32_t, uint32_t) {}
};

} // namespace bpp::IR
