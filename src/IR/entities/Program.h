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
	private:
		OwnedEntityList<Class> classes;
	public:
		void add_diagnostic(std::string, bpp::diagnostic_type, std::string, uint32_t, uint32_t, uint32_t, uint32_t) {}

		bool add_class(std::shared_ptr<Class> class_entity);
		std::shared_ptr<Class> get_class(const std::string& name, size_t max_visible_index = SIZE_MAX) override { return classes.find(name, max_visible_index); }
		std::vector<std::shared_ptr<Class>> get_all_known_classes() const override { return classes.get_entities(); }
		size_t number_of_known_classes() const override { return classes.size(); }
};

} // namespace bpp::IR
