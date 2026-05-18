/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <include/EntityMap.h>
#include <include/BashVersion.h>
#include <AST/Nodes/Program.h>

namespace bpp {

enum class bpp_scope : uint8_t {
	SCOPE_PUBLIC,
	SCOPE_PROTECTED,
	SCOPE_PRIVATE,
	SCOPE_INACCESSIBLE
};

enum class reference_type : uint8_t {
	ref_primitive,
	ref_method,
	ref_object
};

enum class diagnostic_type : uint8_t {
	DIAGNOSTIC_ERROR,
	DIAGNOSTIC_WARNING,
	DIAGNOSTIC_INFO,
	DIAGNOSTIC_HINT
};

/**
 * @struct diagnostic
 * @brief Represents a diagnostic message (error, warning, info, hint)
 *
 * This is used by the language server to report diagnostics to the user.
 * 
 */
struct diagnostic {
	diagnostic_type type;
	std::string message;
	uint32_t start_line;
	uint32_t start_column;
	uint32_t end_line;
	uint32_t end_column;
};

// Forward declarations

class bpp_entity;
class bpp_code_entity;
class bpp_string;
class bpp_method;
class bpp_method_parameter;
class bpp_class;
class bpp_object;
class bpp_datamember;
class bpp_program;

// Statement types
class bash_while_or_until_loop;
class bash_while_or_until_condition;
class bash_if;
class bash_if_branch;
class bash_case;
class bash_case_pattern;
class bash_for_or_select;
class bash_function;
class bpp_delete_statement;
class bpp_dynamic_cast_statement;
class bpp_pointer_dereference;
class bpp_value_assignment;
class bpp_object_assignment;
class bpp_object_reference;
class bpp_object_address;

/**
 * @var bpp_nullptr
 * @brief The secret internal value of '@nullptr' in Bash++
 */
static const char bpp_nullptr[] = "0";

/**
 * @var protected_keywords
 * @brief A list of keywords that are reserved and cannot be used as identifiers in Bash++
 */
inline constexpr std::array<std::string_view, 18> protected_keywords = {
	"class", "constructor", "delete", "destructor",
	"dynamic_cast", "include", "include_once", "local",
	"method", "new", "nullptr","private",
	"protected", "public", "super", "this",
	"typeof", "virtual"
};

/**
 * @brief Check if a string matches any of our protected keywords
 * @param keyword The string to check
 */
inline bool is_protected_keyword(const std::string& keyword) {
	return std::ranges::contains(protected_keywords, keyword);
}

/**
 * @brief Check if a string is a valid identifier in Bash++
 * @param identifier The string to check
 * @return true if the string is a valid identifier, false otherwise
 */
inline bool is_valid_identifier(const std::string& identifier) {
	// Verify it's not empty, and not a reserved keyword
	if (identifier.empty() || is_protected_keyword(identifier)) {
		return false;
	}

	// Verify it doesn't contain two consecutive underscores
	if (identifier.contains("__")) return false;

	// Verify it starts with a letter or underscore, and contains only letters, digits, and underscores
	if (!isalpha(identifier[0]) && identifier[0] != '_') {
		return false;
	}

	for (char c : identifier) {
		if (!isalnum(c) && c != '_') {
			return false;
		}
	}
	
	// If all checks passed, it's a valid identifier
	return true;
}

struct SymbolPosition {
	std::string file;
	uint64_t line = 0;
	uint64_t column = 0;

	SymbolPosition() = default;
	SymbolPosition(const std::string& file, uint64_t line, uint64_t column)
		: file(file), line(line), column(column) {}
};

template <class T>
class OwnedEntityList {
	private:
		std::vector<std::shared_ptr<T>> entities;
		std::unordered_map<std::string, size_t> name_to_index;
	public:
		bool add(std::shared_ptr<T> entity) {
			const std::string& name = entity->get_name();
			if (name_to_index.contains(name)) return false; // Entity with this name already exists
			entities.push_back(entity);
			name_to_index[name] = entities.size() - 1;
			return true;
		}

		std::shared_ptr<T> find(const std::string& name, size_t max_visible_index = SIZE_MAX) {
			auto it = name_to_index.find(name);
			if (it == name_to_index.end()) return nullptr; // No entity with this name
			size_t index = it->second;
			if (index > max_visible_index) return nullptr; // Entity exists but is out of bounds
			return entities[index];
		}

		size_t size() const {
			return entities.size();
		}

		const std::vector<std::shared_ptr<T>>& get_entities() const {
			return entities;
		}
};

} // namespace bpp
