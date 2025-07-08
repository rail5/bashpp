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
}

std::set<std::string> TypeRegistry::get_referenced_types(const nlohmann::json& type_def) const {
	std::set<std::string> referenced;
	const std::string kind = type_def["kind"].get<std::string>();

	if (kind == "reference") {
		const std::string name = type_def["name"].get<std::string>();
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
		return resolve_base_type(type_def["name"].get<std::string>());
	} else if (kind == "reference") {
		const std::string name = type_def["name"].get<std::string>();
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

void TypeRegistry::generate_all_types() const {
	fs::create_directory("generated");
	
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
			std::ofstream file("generated/" + name + ".h");
			file << "#pragma once\n";
			file << "using " << name << " = " 
				 << resolve_type(def["type"]) << ";\n";
		}
	}
}

void TypeRegistry::generate_LSP_types() const {
	// Create LSPAny.h
	std::ofstream any_file("generated/LSPAny.h");
	any_file << R"(
#pragma once
#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <nlohmann/json.hpp>

struct LSPArray;

using LSPAny = std::variant<
	std::unordered_map<std::string, LSPAny>,  // LSPObject
	LSPArray,
	std::string,
	int,
	uint32_t,
	double,
	bool,
	std::nullptr_t
>;

NLOHMANN_JSON_NAMESPACE_BEGIN
template <>
struct adl_serializer<LSPAny> {
	static void to_json(json& j, const LSPAny& any);
	static void from_json(const json& j, LSPAny& any);
};
NLOHMANN_JSON_NAMESPACE_END
)";

	// Create LSPArray.h
	std::ofstream array_file("generated/LSPArray.h");
	array_file << R"(
#pragma once
#include "LSPAny.h"

struct LSPArray : public std::vector<LSPAny> {
	using vector::vector;
};

NLOHMANN_JSON_NAMESPACE_BEGIN
template <>
struct adl_serializer<LSPArray> {
	static void to_json(json& j, const LSPArray& arr) {
		j = static_cast<const std::vector<LSPAny>&>(arr);
	}
	
	static void from_json(const json& j, LSPArray& arr) {
		arr = j.get<std::vector<LSPAny>>();
	}
};
NLOHMANN_JSON_NAMESPACE_END
)";
}

void TypeRegistry::generate_enum(const std::string& name, const nlohmann::json& def) const {
	std::ofstream file("generated/" + name + ".h");
	file << "#pragma once\n";
	file << "#include <string>\n";
	file << "#include <variant>\n";
	file << "#include <vector>\n";
	file << "#include <cstdint>\n";
	file << "#include <nlohmann/json.hpp>\n";
	file << "#include \"LSPAny.h\"\n"; // Include LSPAny for compatibility
	file << "#include \"LSPArray.h\"\n\n"; // Include LSPArray for compatibility
	file << "enum class " << name << " {\n";

	for (const auto& member : def["values"]) {
		file << "	" << member["name"].get<std::string>() << " = "
			 << member["value"] << ",\n";
	}

	file << "};\n\n";
	file << "NLOHMANN_JSON_SERIALIZE_ENUM(" << name << ", {\n";

	for (const auto& member : def["values"]) {
		file << "	{" << name << "::" << member["name"].get<std::string>()
			 << ", \"" << member["name"].get<std::string>() << "\"},\n";
	}

	file << "})\n";
}

void TypeRegistry::generate_struct(const std::string& name, const nlohmann::json& def) const {
	std::ofstream file("generated/" + name + ".h");
	file << "#pragma once\n";
	file << "#include <nlohmann/json.hpp>\n";
	file << "#include \"LSPAny.h\"\n"; // Include LSPAny for compatibility
	file << "#include \"LSPArray.h\"\n"; // Include LSPArray for compatibility

	// Special cases for variant/vector types
	if (name == "LSPAny" || name == "LSPArray") {
		file << "#include <vector>\n";
		file << "#include <variant>\n";
		file << "#include <string>\n";
		file << "#include <unordered_map>\n";
		file << "#include <cstdint>\n";
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

	file << "\nstruct " << name << " {\n";

	// Generate fields
	for (const auto& prop : def["properties"]) {
		const std::string prop_name = prop["name"].get<std::string>();
		const auto& type_def = prop["type"];
		const std::string kind = type_def["kind"].get<std::string>();
		const std::string type_str = resolve_type(type_def);
		file << "    " << type_str << " " << prop_name;
		
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

	file << "\n    NLOHMANN_DEFINE_TYPE_INTRUSIVE(" << name << ",\n";

	// Generate serialization list
	bool first = true;
	for (const auto& prop : def["properties"]) {
		if (!first) file << ",\n";
		file << "        " << prop["name"].get<std::string>();
		first = false;
	}

	file << ")\n};\n" << std::flush;
}
