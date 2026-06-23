/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <cstdint>
#include <stack>

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>

namespace bpp::IR {

class Program : public CodeEntity, public std::enable_shared_from_this<Program> {
	private:
		OwnedEntityList<Class> classes;
	public:
		void add_diagnostic(std::string, bpp::diagnostic_type, std::string, uint32_t, uint32_t, uint32_t, uint32_t) {}

		bool add_class(std::shared_ptr<Class> class_entity);
		std::shared_ptr<Class> get_class(const std::string& name, size_t max_visible_index = SIZE_MAX) override { return classes.find(name, max_visible_index); }
		std::vector<std::shared_ptr<Class>> get_all_known_classes() const override { return classes.get_entities(); }
		size_t number_of_known_classes() const override { return classes.size(); }

		std::weak_ptr<Program> get_containing_program() override { return weak_from_this(); }

		bpp::CodeGen::CodeSegment generate_code() override;

		struct CodeGenState {
			bool in_method = false;
			bool in_class = false;
			std::stack<std::monostate> bash_function_stack;
			std::stack<std::monostate> supershell_stack;
			uint64_t dynamic_cast_counter = 0;
			uint64_t object_counter = 0;

			bool should_declare_local() const {
				return in_class || in_method || !bash_function_stack.empty();
			}

			bool should_localize_object_instantiation() const {
				return should_declare_local() && supershell_stack.empty();
			}
		} codegen_state;
};

} // namespace bpp::IR
