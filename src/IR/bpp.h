// Stub

#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <memory>

namespace bpp {
	enum class diagnostic_type : uint8_t {
		DIAGNOSTIC_ERROR,
		DIAGNOSTIC_WARNING,
		DIAGNOSTIC_INFO,
		DIAGNOSTIC_HINT
	};
}

namespace bpp::IR {

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

// Forward decl. entity types:
class Entity;
class CodeEntity;
class Program;
class Class;
class Method;
class Object;
class DataMember;
class Parameter;

} // namespace bpp::IR
