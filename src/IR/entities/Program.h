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

		//FIXME(@rail5): Stub
		std::shared_ptr<Class> get_class(const std::string& name, size_t max_visible_index = SIZE_MAX) override { return nullptr; }

		//FIXME(@rail5): Stub
		size_t number_of_known_classes() const override { return 0; }
};

} // namespace bpp::IR
