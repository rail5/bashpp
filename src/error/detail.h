/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#pragma once

#include <string_view>
#include <source_location>

namespace bpp_detail {
consteval std::string_view unqualified_function_name(std::string_view function_signature) {
	// Strip it to the identifier after the last "::"
	const auto position = function_signature.rfind("::");
	if (position != std::string_view::npos) function_signature.remove_prefix(position + 2);
	return function_signature;
}
consteval bool starts_with(std::string_view str, std::string_view prefix) {
	return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

consteval bool is_enter_context(std::string_view function_name) {
	if (function_name.find("::enter") != std::string_view::npos) return true;
	return starts_with(unqualified_function_name(function_name), "enter");
}

consteval bool is_exit_context(std::string_view function_name) {
	if (function_name.find("::exit") != std::string_view::npos) return true;
	return starts_with(unqualified_function_name(function_name), "exit");
}

template <class...>
inline constexpr bool false_v = false;
} // namespace bpp_detail
