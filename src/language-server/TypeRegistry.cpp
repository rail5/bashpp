/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "TypeRegistry.h"
#include <fstream>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

void TypeRegistry::load_meta_model(const nlohmann::json& meta_model) {
	// Load all type definitions
	for (const auto& item : meta_model["structures"]) {
		structs[item["name"]] = item;
	}
	for (const auto& item : meta_model["enumerations"]) {
		enums[item["name"]] = item;
	}
	for (const auto& item : meta_model["typeAliases"]) {
		type_aliases[item["name"]] = item;
	}
	for (const auto& item : meta_model["requests"]) {
		requests[item["typeName"]] = item;
	}
}

std::set<std::string> TypeRegistry::get_referenced_types(const nlohmann::json& type_def) const {
	std::set<std::string> referenced;
	const std::string kind = type_def["kind"].get<std::string>();

	if (kind == "reference") {
		const std::string name = get_sanitized_name(type_def["name"].get<std::string>());
		referenced.insert(name);
	} else if (kind == "array") {
		auto nested = get_referenced_types(type_def["element"]);
		referenced.insert(nested.begin(), nested.end());
	} else if (kind == "or") {
		for (const auto& item : type_def["items"]) {
			auto nested = get_referenced_types(item);
			referenced.insert(nested.begin(), nested.end());
		}
	} else if (kind == "map") {
		auto key_refs = get_referenced_types(type_def["key"]);
		auto value_refs = get_referenced_types(type_def["value"]);
		referenced.insert(key_refs.begin(), key_refs.end());
		referenced.insert(value_refs.begin(), value_refs.end());
	}

	return referenced;
}

std::string TypeRegistry::resolve_type(const nlohmann::json& type_def) const {
	return resolve_type(type_def, {});
}

std::string TypeRegistry::resolve_type(const nlohmann::json& type_def, std::set<std::string> visited) const {
	const std::string kind = type_def["kind"].get<std::string>();

	if (kind == "base") {
		return resolve_base_type(get_sanitized_name(type_def["name"].get<std::string>()));
	} else if (kind == "reference") {
		const std::string name = get_sanitized_name(type_def["name"].get<std::string>());
		if (visited.find(name) != visited.end()) {
			return name; // Return name for forward declaration
		}
		visited.insert(name);
		return resolve_reference_type(name, visited);
	} else if (kind == "array") {
		return resolve_array_type(type_def, visited);
	} else if (kind == "or") {
		return resolve_or_type(type_def, visited);
	} else if (kind == "map") {
		return resolve_map_type(type_def, visited);
	} else if (kind == "stringLiteral") {
		return "std::string"; // Special handling for string literals
	} else if (kind == "booleanLiteral") {
		return "bool"; // Special handling for boolean literals
	} else if (kind == "integerLiteral") {
		return "int"; // Special handling for integer literals
	} else if (kind == "literal") {
		return resolve_literal_type(type_def, visited);
	} else if (kind == "tuple") {
		return resolve_tuple_type(type_def, visited);
	}
	return "UnknownType_" + kind;
}

std::string TypeRegistry::resolve_base_type(const std::string& name) const {
	static const std::unordered_map<std::string, std::string> base_types = {
		{"URI", "std::string"},
		{"DocumentUri", "std::string"},
		{"integer", "int"},
		{"uinteger", "uint32_t"},
		{"decimal", "double"},
		{"RegExp", "std::string"},
		{"string", "std::string"},
		{"create", "std::string"},
		{"boolean", "bool"},
		{"null", "std::nullptr_t"}
	};

	auto it = base_types.find(name);
	return it != base_types.end() ? it->second : "UnknownType_" + name;
}

std::string TypeRegistry::resolve_reference_type(const std::string& name, std::set<std::string> visited) const {
	if (name == "LSPObject") {
		return "std::unordered_map<std::string, LSPAny>";
	}
	if (structs.find(name) != structs.end()) return name;
	if (enums.find(name) != enums.end()) return name;
	if (type_aliases.find(name) != type_aliases.end()) {
		return resolve_type(type_aliases.at(name)["type"], visited);
	}
	return "UnresolvedType_" + name;
}

std::string TypeRegistry::resolve_array_type(const nlohmann::json& type_def, std::set<std::string> visited) const {
	return "std::vector<" + resolve_type(type_def["element"], visited) + ">";
}

std::string TypeRegistry::resolve_or_type(const nlohmann::json& type_def, std::set<std::string> visited) const {
	std::string result = "std::variant<";
	bool first = true;
	for (const auto& item : type_def["items"]) {
		if (!first) result += ", ";
		result += resolve_type(item, visited);
		first = false;
	}
	result += ">";
	return result;
}

std::string TypeRegistry::resolve_map_type(const nlohmann::json& type_def, std::set<std::string> visited) const {
	const std::string key_type = resolve_type(type_def["key"], visited);
	const std::string value_type = resolve_type(type_def["value"], visited);
	return "std::unordered_map<" + key_type + ", " + value_type + ">";
}

std::string TypeRegistry::resolve_literal_type(const nlohmann::json& type_def, std::set<std::string> visited) const {
	// Literal types represent inline objects
	if (type_def.contains("value") && type_def["value"].contains("properties")) {
		const auto& props = type_def["value"]["properties"];
		if (props.empty()) {
			// Empty object literal
			return "std::monostate";
		} else {
			// For non-empty literals, we need to generate a struct
			// This is complex - we'll handle common cases instead
			return "nlohmann::json"; // Fallback to generic JSON
		}
	}
	return "nlohmann::json"; // Default to generic JSON
}

std::string TypeRegistry::resolve_tuple_type(const nlohmann::json& type_def, std::set<std::string> visited) const {
	std::string tuple_type = "std::tuple<";
	bool first = true;

	for (const auto& item : type_def["items"]) {
		if (!first) tuple_type += ", ";
		tuple_type += resolve_type(item, visited);
		first = false;
	}

	tuple_type += ">";
	return tuple_type;
}

std::vector<std::string> TypeRegistry::get_base_classes(const nlohmann::json& def) const {
	std::vector<std::string> bases;

	// Handle extends
	if (def.contains("extends")) {
		for (const auto& base : def["extends"]) {
			bases.push_back(resolve_type(base));
		}
	}

	// Handle mixins
	if (def.contains("mixins")) {
		for (const auto& mixin : def["mixins"]) {
			bases.push_back(resolve_type(mixin));
		}
	}

	return bases;
}

void TypeRegistry::generate_inheritance(std::ofstream& file, const std::vector<std::string>& base_classes) const {
	if (!base_classes.empty()) {
		file << " : ";
		for (size_t i = 0; i < base_classes.size(); ++i) {
			if (i > 0) file << ", ";
			file << "public " << base_classes[i];
		}
	}
}

std::string TypeRegistry::get_variant_deserialization_code(
	const std::string& prop_name, 
	const std::string& variant_type,
	bool is_optional
) const {
	// Extract types between < and >
	size_t start = variant_type.find('<');
	size_t end = variant_type.rfind('>');
	if (start == std::string::npos || end == std::string::npos) {
		return "// Error: malformed variant type\n";
	}

	std::string inner = variant_type.substr(start + 1, end - start - 1);
	std::vector<std::string> types;
	std::string current;
	int depth = 0;

	// Parse types considering nested templates
	for (char c : inner) {
		if (c == '<') depth++;
		if (c == '>') depth--;
		
		if (c == ',' && depth == 0) {
			if (!current.empty()) {
				types.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}
	if (!current.empty()) types.push_back(current);

	// Generate deserialization code
	std::string code;
	std::set<std::string> conditions_used;

	for (const auto& type : types) {
		std::string condition;
		std::string getter;
		
		// Trim whitespace
		std::string clean_type = type;
		clean_type.erase(0, clean_type.find_first_not_of(" \t"));
		clean_type.erase(clean_type.find_last_not_of(" \t") + 1);
		
		// Map types to JSON conditions
		if (clean_type == "std::string") {
			condition = "is_string()";
		} else if (clean_type == "int") {
			condition = "is_number_integer()";
		} else if (clean_type == "uint32_t") {
			condition = "is_number_unsigned()";
		} else if (clean_type == "double") {
			condition = "is_number_float()";
		} else if (clean_type == "bool") {
			condition = "is_boolean()";
		} else if (clean_type == "std::nullptr_t") {
			condition = "is_null()";
		} else if (clean_type == "LSPArray" || 
					clean_type.find("std::vector") != std::string::npos) {
			condition = "is_array()";
		} else if (clean_type == "LSPObject" || 
					clean_type.find("std::unordered_map") != std::string::npos) {
			condition = "is_object()";
		} else {
			// Assume other types are objects
			condition = "is_object()";
		}
		
		// Only add condition if not already used
		if (conditions_used.find(condition) == conditions_used.end()) {
			conditions_used.insert(condition);
			
			if (clean_type == "std::nullptr_t") {
				getter = "obj." + prop_name + " = nullptr;";
			} else {
				getter = "obj." + prop_name + " = j[\"" + prop_name + "\"].get<" + clean_type + ">();";
			}
			
			if (code.empty()) {
				code = "if (j[\"" + prop_name + "\"]." + condition + ") {\n";
			} else {
				code += "} else if (j[\"" + prop_name + "\"]." + condition + ") {\n";
			}
			code += "    " + getter + "\n";
		}
	}

	if (!code.empty()) {
		code += "} else {\n";
		code += "    throw std::runtime_error(\"Unexpected type for property " + prop_name + "\");\n";
		code += "}";
		
		if (is_optional) {
			return "if (j.contains(\"" + prop_name + "\")) {\n" + code + "\n}\n";
		}
		return code + "\n";
	}

	return "// Could not generate variant deserialization\n";
}

void TypeRegistry::generate_serialization(std::ofstream& file, 
		const std::string& name,
		const std::vector<std::string>& base_classes,
		const nlohmann::json& properties) const {
	file << "	friend void to_json(nlohmann::json& j, const " << name << "& obj) {\n";
	file << "		j = nlohmann::json::object();\n";

	// Serialize base classes
	for (const auto& base : base_classes) {
		file << "		nlohmann::json " << base << "_json = static_cast<const " << base << "&>(obj);\n";
		file << "		j.update(" << base << "_json);\n";
	}

	// Serialize direct properties
	for (const auto& prop : properties) {
		const std::string prop_name = get_sanitized_name(prop["name"].get<std::string>());
		// It might be a std::variant
		// Is it a std::variant?

		// Do not serialize optional properties if they are not present
		bool is_optional = prop.value("optional", false);

		if (prop.contains("type") && resolve_type(prop["type"]).find("std::variant") == 0) {
			if (is_optional) {
				file << "		if (obj." << prop_name << ".has_value()) {\n";
				file << "			std::visit([&j](auto&& value) {\n";
				file << "				j[\"" << prop_name << "\"] = value;\n";
				file << "			}, *obj." << prop_name << ");\n";
				file << "		}\n";
			} else {
				file << "		std::visit([&j](auto&& value) {\n"
					<< "			j[\"" << prop_name << "\"] = value;\n"
					<< "		}, obj." << prop_name << ");\n";
			}
		} else {
			if (is_optional) {
				file << "		if (obj." << prop_name << " != std::nullopt) {\n";
				file << "			j[\"" << prop_name << "\"] = *";
			} else {
				file << "		j[\"" << prop_name << "\"] = ";
			}

			if (prop.contains("type") && resolve_type(prop["type"]) == name) {
				// de-reference,
				// Since if the struct contains an instance of itself,
				// We'll have used a shared_ptr
				file << "*";
			}
			file << "obj." << prop_name << ";\n";

			if (is_optional) {
				file << "		}\n";
			}
		}
	}

	file << "	}\n\n";

	file << "	friend void from_json(const nlohmann::json& j, " << name << "& obj) {\n";

	// Deserialize base classes
	for (const auto& base : base_classes) {
		file << "		{\n";
		file << "			" << base << " base_obj;\n";
		file << "			j.get_to(base_obj);\n";
		file << "			static_cast<" << base << "&>(obj) = base_obj;\n";
		file << "		}\n";
	}

	// Deserialize direct properties
	for (const auto& prop : properties) {
		const std::string prop_name = get_sanitized_name(prop["name"].get<std::string>());
		const bool is_optional = prop.value("optional", false);

		std::string get_to_str;

		// Is it a std::variant?
		std::string type_str = resolve_type(prop["type"]);
		if (type_str.find("std::variant") == 0) {
			file << get_variant_deserialization_code(prop_name, type_str, is_optional);
		} else {
			// Normal property handling
			if (is_optional) {
				file << "		if (j.contains(\"" << prop_name << "\")) {\n";
				file << "			if (j[\"" << prop_name << "\"].is_null()) {\n";
				file << "				obj." << prop_name << " = std::nullopt;\n";
				file << "			} else {\n";
				if (type_str == name) {
					file << "				obj." << prop_name << " = std::make_shared<" << name << ">(j[\"" << prop_name << "\"].get<" << type_str << ">());\n";
				} else {
					file << "				obj." << prop_name << " = j[\"" << prop_name << "\"].get<" << type_str << ">();\n";
				}
				file << "			}\n";
				file << "		} else {\n";
				file << "			obj." << prop_name << " = std::nullopt;\n";
				file << "		}\n";
			} else {
				file << "		j[\"" << prop_name << "\"].get_to(obj." << prop_name << ");\n";
			}
		}
	}

	file << "	}\n";
}

void TypeRegistry::generate_all_types() const {
	fs::create_directory(output_directory);
	
	// Generate special types first
	generate_LSP_types();
	
	// Generate enums
	for (const auto& [name, def] : enums) {
		if(name != "LSPAny" && name != "LSPArray") {
			generate_enum(name, def);
		}
	}
	
	// Generate structs
	for (const auto& [name, def] : structs) {
		if(name != "LSPAny" && name != "LSPArray") {
			generate_struct(name, def);
		}
	}
	
	// Generate type aliases
	for (const auto& [name, def] : type_aliases) {
		if(name != "LSPAny" && name != "LSPArray") {
			generate_type_alias(name, def);
		}
	}

	// Generate requests
	for (const auto& [name, def] : requests) {
		generate_request(name, def);
	}
}

void TypeRegistry::generate_LSP_types() const {
	std::ofstream file_any(output_directory + "/LSPAny.h");
	file_any << "#pragma once\n";
	file_any << "#include \"../static/LSPTypes.h\"\n" << std::flush;
	file_any.close();

	std::ofstream file_array(output_directory + "/LSPArray.h");
	file_array << "#pragma once\n";
	file_array << "#include \"../static/LSPTypes.h\"\n" << std::flush;
	file_array.close();
}

std::string TypeRegistry::get_sanitized_name(const std::string& name) const {
	static const std::set<std::string> reserved_keywords = {
		"int", "float", "double", "char", "void", "bool", "if", "else",
		"for", "while", "return", "class", "struct", "enum", "export",
		"static", "public", "private", "protected", "virtual",
		"override", "const", "constexpr", "inline", "namespace", "using",
		"template", "typename", "this", "new", "delete", "try", "catch",
		"throw", "switch", "case", "default", "break", "continue",
		"operator"
	};
	if (reserved_keywords.find(name) != reserved_keywords.end()) {
		return name + "_"; // Append underscore to avoid conflicts with keywords
	}
	return name;
}

void TypeRegistry::generate_enum(const std::string& name, const nlohmann::json& def) const {
	std::ofstream file(output_directory + "/" + name + ".h");
	file << "#pragma once\n";
	file << "#include <string>\n";
	file << "#include <variant>\n";
	file << "#include <vector>\n";
	file << "#include <cstdint>\n";
	file << "#include <optional>\n";
	file << "#include <memory>\n";
	file << "#include <nlohmann/json.hpp>\n";
	file << "#include \"../static/LSPTypes.h\"\n"; // Include LSPTypes for compatibility

	// LSP MetaModel lists static string constants as "enumerations"
	// Obviously these are not representable as enums in C++
	if (def.contains("type") && def["type"]["name"] == "string") {

		// Doxygen comments
		file << "/**\n";
		file << " * @class " << name << "\n";
		std::string brief = def.value("documentation", "No description provided.");
		// Remove any '@' symbols from the brief
		brief.erase(std::remove(brief.begin(), brief.end(), '@'), brief.end());
		file << " * @brief " << brief << "\n";
		file << " **/\n";

		file << "class " << name << " {\n";
		file << "private:\n";
		file << "	std::string value_;\n";
		file << "public:\n";
		for (const auto& member : def["values"]) {
			file << "	static const " << name << " "
				<< get_sanitized_name(member["name"].get<std::string>()) << ";\n";
		}
		file << "\n	" << name << "() = default;\n";
		file << "	explicit " << name << "(const std::string& value) : value_(value) {}\n";
		file << "	const std::string& value() const { return value_; }\n";
		file << "	bool operator==(const " << name << "& other) const { return value_ == other.value_; }\n";
		file << "	bool operator!=(const " << name << "& other) const { return !(*this == other); }\n";
		file << "};\n\n";

		for (const auto& member : def["values"]) {
			file << "const " << name << " " << name << "::"
				 << get_sanitized_name(member["name"].get<std::string>()) << " = "
				 << name << "(\"" << get_sanitized_name(member["value"].get<std::string>()) << "\");\n";
		}

		file << "\ninline void to_json(nlohmann::json& j, const " << name << "& obj) {\n";
		file << "	j = obj.value();\n";
		file << "}\n\n";
		file << "inline void from_json(const nlohmann::json& j, " << name << "& obj) {\n";
		file << "	obj = " << name << "(j.get<std::string>());\n";
		file << "}\n\n";
		return; // Special case handled
	}

	// Doxygen comments
	file << "/**\n";
	file << " * @enum " << name << "\n";
	std::string brief = def.value("documentation", "No description provided.");
	// Remove any '@' symbols from the brief
	brief.erase(std::remove(brief.begin(), brief.end(), '@'), brief.end());
	file << " * @brief " << brief << "\n";
	file << " **/\n";

	file << "enum class " << name << " {\n";

	for (const auto& member : def["values"]) {
		file << "	" << get_sanitized_name(member["name"].get<std::string>()) << " = "
			 << member["value"] << ",\n";
	}

	file << "};\n\n";
	file << "NLOHMANN_JSON_SERIALIZE_ENUM(" << name << ", {\n";

	for (const auto& member : def["values"]) {
		file << "	{" << name << "::" << get_sanitized_name(member["name"].get<std::string>())
			 << ", " << std::to_string(member["value"].get<int>()) << "},\n";
	}

	file << "})\n";
}

void TypeRegistry::generate_type_alias(const std::string& name, const nlohmann::json& def) const {
	std::ofstream file(output_directory + "/" + name + ".h");
	file << "#pragma once\n";
	file << "#include <nlohmann/json.hpp>\n";
	file << "#include <optional>\n";
	file << "#include <memory>\n";
	file << "#include \"../static/LSPTypes.h\"\n"; // Include LSPTypes for compatibility
	
	std::set<std::string> includes;
	auto refs = get_referenced_types(def["type"]);
	for (const auto& ref : refs) {
		// Skip base types
		static const std::set<std::string> base_types = {
			"string", "integer", "uinteger", "decimal", "boolean", "null"
		};

		if (base_types.find(ref) == base_types.end()) {
			includes.insert("#include \"" + ref + ".h\"");
		}
	}

	// Write includes
	for (const auto& inc : includes) {
		file << inc << "\n";
	}

	// Doxygen comments
	file << "/**\n";
	file << " * @struct " << name << "\n";
	std::string brief = def.value("documentation", "No description provided.");
	// Remove any '@' symbols from the brief
	brief.erase(std::remove(brief.begin(), brief.end(), '@'), brief.end());
	file << " * @brief " << brief << "\n";
	file << " **/\n";

	file << "using " << name << " = " << resolve_type(def["type"]) << ";\n";
}

void TypeRegistry::generate_struct(const std::string& name, const nlohmann::json& def) const {
	std::ofstream file(output_directory + "/" + name + ".h");
	file << "#pragma once\n";
	file << "#include <nlohmann/json.hpp>\n";
	file << "#include \"../static/LSPTypes.h\"\n"; // Include LSPTypes for compatibility

	// Special cases for variant/vector types
	if (name == "LSPAny" || name == "LSPArray") {
		file << "#include <vector>\n";
		file << "#include <variant>\n";
		file << "#include <string>\n";
		file << "#include <unordered_map>\n";
		file << "#include <cstdint>\n";
		file << "#include <optional>\n";
		file << "#include <memory>\n";
	}

	auto base_classes = get_base_classes(def);
	for (const auto& base : base_classes) {
		file << "#include \"" << base << ".h\"\n";
	}

	// Collect all referenced types
	std::set<std::string> includes;
	for (const auto& prop : def["properties"]) {
		auto refs = get_referenced_types(prop["type"]);
		for (const auto& ref : refs) {
			// Skip base types
			static const std::set<std::string> base_types = {
				"string", "integer", "uinteger", "decimal", "boolean", "null"
			};
			
			if (base_types.find(ref) == base_types.end()) {
				includes.insert("#include \"" + ref + ".h\"");
			}
		}
	}

	// Write includes
	for (const auto& inc : includes) {
		file << inc << "\n";
	}

	// Generate Doxygen-style comments
	file << "/**\n";
	file << " * @file " << name << ".h\n";
	file << " * @struct " << name << "\n";
	std::string brief = def.value("documentation", "No description provided.");
	// Remove any '@' symbols from the brief
	brief.erase(std::remove(brief.begin(), brief.end(), '@'), brief.end());
	file << " * @brief " << brief << "\n";
	file << " **/\n";

	file << "struct " << name;
	generate_inheritance(file, base_classes);
	file << " {\n";

	// Generate fields
	for (const auto& prop : def["properties"]) {
		const std::string prop_name = get_sanitized_name(prop["name"].get<std::string>());
		const auto& type_def = prop["type"];
		const std::string kind = type_def["kind"].get<std::string>();
		std::string type_str = resolve_type(type_def);
		bool is_optional = prop.value("optional", false);
		if (is_optional) {
			type_str = "std::optional<" + type_str + ">";
		}

		// Check if the type is the same as the containing struct
		// E.g.: SelectionRange, per the spec, has a property 'parent' of type SelectionRange
		if (type_str == name) {
			type_str = name + "&"; // Use reference to avoid infinite recursion
		} else if (type_str == "std::optional<" + name + ">") {
			type_str = "std::optional<std::shared_ptr<" + name + ">>"; // Use pointer for optional, unions cannot contain reference types
		}

		file << "	/**\n";
		file << "	 * @brief (";
		std::string type_description = type_def.value("documentation", "No description provided.");
		// Remove any '@' symbols from the description
		type_description.erase(std::remove(type_description.begin(), type_description.end(), '@'), type_description.end());
		file << (prop.contains("optional") && prop["optional"].get<bool>() ? "Optional" : "Required") << ") ";
		file << type_description << "\n";
		file << "	 **/\n";

		file << "	" << type_str << " " << prop_name;
		
		// Add default values for literal types
		if (kind == "stringLiteral") {
			file << " = \"" << type_def["value"].get<std::string>() << "\"";
		} else if (kind == "booleanLiteral") {
			file << " = " << (type_def["value"].get<bool>() ? "true" : "false");
		} else if (kind == "integerLiteral") {
			file << " = " << type_def["value"].get<int>();
		}
		
		file << ";\n";
	}

	generate_serialization(file, name, base_classes, def["properties"]);

	file << "};\n" << std::flush;
}

void TypeRegistry::generate_request(const std::string& name, const nlohmann::json& def) const {
	std::ofstream file(output_directory + "/" + name + ".h");
	file << "#pragma once\n";
	file << "#include <string>\n";
	file << "#include <variant>\n";
	file << "#include <vector>\n";
	file << "#include <unordered_map>\n";
	file << "#include <cstdint>\n";
	file << "#include <optional>\n";
	file << "#include <memory>\n";
	file << "#include <nlohmann/json.hpp>\n";
	file << "#include \"../static/LSPTypes.h\"\n"; // Include LSPTypes for compatibility
	file << "#include \"../static/Message.h\"\n"; // Include Message for base classes

	// Include any necessary headers for referenced types in params and response
	std::set<std::string> includes;
	if (def.contains("params")) {
		includes = get_referenced_types(def["params"]);
	}

	if (def.contains("result")) {
		auto response_refs = get_referenced_types(def["result"]);
		includes.insert(response_refs.begin(), response_refs.end());
	}

	// Write includes
	for (const auto& inc : includes) {
		// Skip base types
		static const std::set<std::string> base_types = {
			"string", "integer", "uinteger", "decimal", "boolean", "null"
		};

		if (base_types.find(inc) == base_types.end()) {
			file << "#include \"" << inc << ".h\"\n";
		}
	}

	// We have to generate both the Request and Response types

	// Handle the request type
	// What kind of params are permissible?
	/**
	 * The JSON format for these is:
	 * 		"params": {
	 * 			"kind": "reference",
	 * 			"name": "SelectionRange"
	 * 		}
	 * 
	 * Each listing in the spec has *only one* entry in "params"
	 */
	file << "/**\n";
	file << " * @file " << name << ".h\n";
	file << " * @struct " << name << "\n";

	std::string brief = def.value("documentation", "No description provided.");
	// Remove any '@' symbols from the brief
	brief.erase(std::remove(brief.begin(), brief.end(), '@'), brief.end());
	file << " * @brief " << brief << "\n";
	file << " **/\n";
	
	if (def.contains("params")) {
		std::string params_type = resolve_type(def["params"]);
		file << "using " << name << " = RequestMessage<" << params_type << ">;\n";
	}

	// Handle the response type
	// Same deal, essentially
	if (def.contains("result")) {
		std::string response_type = resolve_type(def["result"]);
		file << "using " << name << "Response = ResponseMessage<" << response_type << ">;\n";
	}

	// The serialization is handled in the template base classes

	file << std::flush;
	file.close();
}
