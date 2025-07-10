#pragma once
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
	template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, LSPAny>>>
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

// Generic serializer for std::variant
template <typename... Args>
struct adl_serializer<std::variant<Args...>> {
	static void to_json(json& j, const std::variant<Args...>& v) {
		std::visit([&j](const auto& value) {
			j = value;
		}, v);
	}

	static void from_json(const json& j, std::variant<Args...>& v) {
		bool found = false;
		// Iterate over all types in the variant using a fold expression
		( ( [&] {
			// If a valid type has already been found, skip further checks
			if (found) return;
			try {
				// Attempt to deserialize the JSON object into the current type
				v = j.get<Args>();
				// Mark as found if successful
				found = true;
			} catch (...) {
				// Ignore exceptions and continue checking other types
			}
		})(), ... );
		
		// If no valid type was found, throw an error indicating deserialization failure
		if (!found) {
			throw std::runtime_error("Could not deserialize variant");
		}
	}
};

// Serializer for std::monostate
template <>
struct adl_serializer<std::monostate> {
	static void to_json(json& j, const std::monostate&) {
		j = nullptr; // Represent std::monostate as null in JSON
	}
	static void from_json(const json& j, std::monostate&) {
		if (!j.is_null()) {
			throw std::runtime_error("Expected null for std::monostate");
		}
		// No action needed, std::monostate is already default constructed
	}
};

NLOHMANN_JSON_NAMESPACE_END
