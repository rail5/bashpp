/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <ranges>

#include <include/EntityMap.h>
#include <include/BashVersion.h>

#include "bpp.h"
#include "bpp_code_entity.h"

namespace bpp {

/**
 * @class bpp_program
 * 
 * @brief The main program
 */
class bpp_program : public bpp_code_entity, public std::enable_shared_from_this<bpp_program> {
	private:
		uint64_t supershell_counter = 0;
		uint64_t assignment_counter = 0;
		uint64_t function_counter = 0;
		uint64_t dynamic_cast_counter = 0;
		uint64_t typeof_counter = 0;
		uint64_t object_counter = 0;
		
		BashVersion target_bash_version = {5, 2};

		std::string main_source_file;

		// To ensure that the bpp_program **owns** its classes
		// I.e., that those classes don't get destroyed before we're done with them
		OwnedEntityList<bpp_class> owned_classes;

		// Source file -> AST
		// Used for advanced analysis, e.g. LSP features
		std::unordered_map<std::string, std::shared_ptr<AST::Program>> source_file_asts;

		// Source file -> EntityMap
		std::unordered_map<std::string, EntityMap> entity_maps;
		// s.t. requesting entity_maps["/path/to/file1.bpp"] returns an EntityMap
		// which outlines for us which container entities are active at each point in the file

		// Source file -> Diagnostics
		std::unordered_map<std::string, std::vector<bpp::diagnostic>> diagnostics;
		// Each 'diagnostic' contains a type (error, warning, etc), message, and position in the source file

		// For debug info:
		std::shared_ptr<std::vector<std::string>> include_paths;
	public:
		bpp_program() = default;
		~bpp_program() override = default;

		bpp_program(const bpp_program& other) = default;
		bpp_program& operator=(const bpp_program& other) = default;
		bpp_program(bpp_program&& other) noexcept = default;
		bpp_program& operator=(bpp_program&& other) noexcept = default;

		bool set_containing_class(std::weak_ptr<bpp_class> containing_class) override;
		void set_output_stream(std::shared_ptr<std::ostream> output_stream);

		bool prepare_class(std::shared_ptr<bpp_class> class_);
		bool add_class(std::shared_ptr<bpp_class> class_);

		std::shared_ptr<bpp_class> get_class(const std::string& name, size_t max_visible_index = SIZE_MAX) override;

		std::vector<std::shared_ptr<bpp_class>> get_all_known_classes() const override;
		size_t number_of_known_classes() const override;

		std::weak_ptr<bpp_program> get_containing_program() override;

		void set_include_paths(std::shared_ptr<std::vector<std::string>> paths);
		std::shared_ptr<std::vector<std::string>> get_include_paths() const;

		void increment_supershell_counter();
		uint64_t get_supershell_counter() const;

		void increment_assignment_counter();
		uint64_t get_assignment_counter() const;

		void increment_function_counter();
		uint64_t get_function_counter() const;

		void increment_dynamic_cast_counter();
		uint64_t get_dynamic_cast_counter() const;

		void increment_typeof_counter();
		uint64_t get_typeof_counter() const;

		void increment_object_counter();
		uint64_t get_object_counter() const;

		void set_target_bash_version(BashVersion target_bash_version);
		BashVersion get_target_bash_version() const;

		void mark_entity(
			const std::string& file,
			uint32_t start_line, uint32_t start_column,
			uint32_t end_line, uint32_t end_column,
			std::shared_ptr<bpp::bpp_entity> entity
		);

		std::shared_ptr<bpp::bpp_entity> get_active_entity(
			const std::string& file,
			uint32_t line, uint32_t column
		);

		auto get_source_files() const { return entity_maps | std::views::keys; }
		const std::string& get_main_source_file() const;
		void set_main_source_file(const std::string& file);
		void add_source_file(const std::string& file);

		void set_source_file_ast(const std::string& file, std::shared_ptr<AST::Program> ast);
		std::shared_ptr<AST::Program> get_source_file_ast(const std::string& file) const;

		void add_diagnostic(
			const std::string& file,
			diagnostic_type type,
			const std::string& message,
			uint32_t start_line, uint32_t start_column,
			uint32_t end_line, uint32_t end_column
		);

		std::vector<bpp::diagnostic> get_diagnostics(const std::string& file) const;
		void clear_diagnostics(const std::string& file);
};

} // namespace bpp
