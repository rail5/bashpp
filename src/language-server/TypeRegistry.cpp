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
		const std::string prop_name = prop["name"].get<std::string>();
		file << "		j[\"" << prop_name << "\"] = obj." << prop_name << ";\n";
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
		const std::string prop_name = prop["name"].get<std::string>();
		const bool is_optional = prop.value("optional", false);
		
		if (is_optional) {
			file << "		if (j.contains(\"" << prop_name << "\")) {\n";
			file << "			j[\"" << prop_name << "\"].get_to(obj." << prop_name << ");\n";
			file << "		}\n";
		} else {
			file << "		j.at(\"" << prop_name << "\").get_to(obj." << prop_name << ");\n";
		}
	}

	file << "	}\n";
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
	std::ofstream file("generated/LSPTypes.h");
	file << R"(#pragma once
#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <nlohmann/json.hpp>

// Recursive wrapper for variant types
template <typename T>
struct RecursiveWrapper {
	T value;
	
	RecursiveWrapper() = default;
	RecursiveWrapper(const T& v) : value(v) {}
	RecursiveWrapper(T&& v) : value(std::move(v)) {}
	
	operator T&() { return value; }
	operator const T&() const { return value; }
};

// Forward declarations
struct LSPAny;
using LSPArray = std::vector<LSPAny>;

// LSPAny definition
struct LSPAny {
	using ValueType = std::variant<
		std::unordered_map<std::string, LSPAny>,
		RecursiveWrapper<LSPArray>,
		std::string,
		int,
		uint32_t,
		double,
		bool,
		std::nullptr_t
	>;
	
	ValueType value;

	// Constructors
	LSPAny() : value(nullptr) {}
	template <typename T>
	LSPAny(T&& val) : value(std::forward<T>(val)) {}
	
	// Accessor
	template <typename T>
	const T* get_if() const {
		return std::get_if<T>(&value);
	}
};

// Serialization support
NLOHMANN_JSON_NAMESPACE_BEGIN

template <typename T>
struct adl_serializer<RecursiveWrapper<T>> {
	static void to_json(json& j, const RecursiveWrapper<T>& wrapper) {
		j = wrapper.value;
	}
	
	static void from_json(const json& j, RecursiveWrapper<T>& wrapper) {
		wrapper.value = j.get<T>();
	}
};

template <>
struct adl_serializer<LSPAny> {
	static void to_json(json& j, const LSPAny& any) {
		std::visit([&](auto&& arg) {
			j = arg;
		}, any.value);
	}
	
	static void from_json(const json& j, LSPAny& any) {
		if (j.is_object()) {
			any.value = j.get<std::unordered_map<std::string, LSPAny>>();
		} else if (j.is_array()) {
			any.value = RecursiveWrapper<LSPArray>(j.get<LSPArray>());
		} else if (j.is_string()) {
			any.value = j.get<std::string>();
		} else if (j.is_number_integer()) {
			any.value = j.get<int>();
		} else if (j.is_number_unsigned()) {
			any.value = j.get<uint32_t>();
		} else if (j.is_number_float()) {
			any.value = j.get<double>();
		} else if (j.is_boolean()) {
			any.value = j.get<bool>();
		} else if (j.is_null()) {
			any.value = nullptr;
		}
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
	file << "#include \"LSPTypes.h\"\n"; // Include LSPTypes for compatibility
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
	file << "#include \"LSPTypes.h\"\n"; // Include LSPTypes for compatibility

	// Special cases for variant/vector types
	if (name == "LSPAny" || name == "LSPArray") {
		file << "#include <vector>\n";
		file << "#include <variant>\n";
		file << "#include <string>\n";
		file << "#include <unordered_map>\n";
		file << "#include <cstdint>\n";
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

	file << "\nstruct " << name;
	generate_inheritance(file, base_classes);
	file << " {\n";

	// Generate fields
	for (const auto& prop : def["properties"]) {
		const std::string prop_name = prop["name"].get<std::string>();
		const auto& type_def = prop["type"];
		const std::string kind = type_def["kind"].get<std::string>();
		std::string type_str = resolve_type(type_def);

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
