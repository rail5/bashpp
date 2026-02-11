/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */
#pragma once
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <nlohmann/json.hpp>

class TypeRegistry {
	private:
		std::string output_directory = "generated";

		std::unordered_map<std::string, nlohmann::json> structs;
		std::unordered_map<std::string, nlohmann::json> enums;
		std::unordered_map<std::string, nlohmann::json> type_aliases;
		std::unordered_map<std::string, nlohmann::json> requests;
		std::unordered_map<std::string, nlohmann::json> notifications;

		std::string get_sanitized_name(const std::string& name) const;
		std::string get_sanitized_description(const std::string& description) const;

		std::string resolve_base_type(const std::string& name) const;
		std::string resolve_reference_type(const std::string& name, std::set<std::string> visited) const;
		std::string resolve_array_type(const nlohmann::json& type_def, std::set<std::string> visited) const;
		std::string resolve_or_type(const nlohmann::json& type_def, std::set<std::string> visited) const;
		std::string resolve_map_type(const nlohmann::json& type_def, std::set<std::string> visited) const;
		std::string resolve_literal_type(const nlohmann::json& type_def, std::set<std::string> visited) const;
		std::string resolve_tuple_type(const nlohmann::json& type_def, std::set<std::string> visited) const;

		std::vector<std::string> get_base_classes(const nlohmann::json& def) const;
		void generate_inheritance(std::ofstream& file, const std::vector<std::string>& base_classes) const;
		std::set<std::string> get_referenced_types(const nlohmann::json& type_def) const;

		std::string get_variant_deserialization_code(
			const std::string& prop_name, 
			const std::string& variant_type,
			bool is_optional) const;

		void generate_serialization(std::ofstream& file, 
			const std::string& name,
			const std::vector<std::string>& base_classes,
			const nlohmann::json& properties) const;

		void generate_LSP_types() const;
		void generate_struct(const std::string& name, const nlohmann::json& def) const;
		void generate_enum(const std::string& name, const nlohmann::json& def) const;
		void generate_type_alias(const std::string& name, const nlohmann::json& def) const;
		void generate_request(const std::string& name, const nlohmann::json& def) const;
		void generate_notification(const std::string& name, const nlohmann::json& def) const;
	public:
		void load_meta_model(const nlohmann::json& meta_model);
		std::string resolve_type(const nlohmann::json& type_def) const;
		std::string resolve_type(const nlohmann::json& type_def, std::set<std::string> visited) const;
		void generate_all_types() const;

		void set_output_directory(const std::string& dir) {
			output_directory = dir;
		}
};
