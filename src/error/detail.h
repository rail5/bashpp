/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#pragma once

#include <string_view>
#include <source_location>
#include <type_traits>
#include <concepts>
#include <stack>

#include <AST/ASTNode.h>
#include <AST/Nodes/Nodes.h>
#include <bpp_include/bpp.h>

namespace bpp {
namespace detail {
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

template <typename T>
concept ASTNodePtrType = std::is_same_v<std::shared_ptr<AST::ASTNode>, T> ||
	std::is_base_of_v<AST::ASTNode, typename T::element_type>;

template <typename T>
concept ASTStringToken = std::is_same_v<AST::Token<std::string>, T>;

template <typename T>
concept ASTParameterToken = std::is_same_v<AST::Token<AST::MethodDefinition::Parameter>, T>;

template <typename T>
concept ASTNodePtrORToken = ASTNodePtrType<T> || ASTStringToken<T> || ASTParameterToken<T>;

template <typename T>
concept ErrorReportableListener = requires(T t) {
	{ t.get_program() } -> std::same_as<std::shared_ptr<bpp::bpp_program>>;
	{ t.get_include_stack() } -> std::same_as<std::stack<std::string>>;
	{ t.get_source_file() } -> std::same_as<std::string>;
	{ t.get_lsp_mode() } -> std::same_as<bool>;
	{ t.set_has_errors(true) } -> std::same_as<void>;
};

} // namespace detail
} // namespace bpp
