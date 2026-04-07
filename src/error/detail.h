/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <type_traits>
#include <concepts>

#include <AST/ASTNode.h>
#include <AST/Nodes/Nodes.h>
#include <bpp_include/bpp.h>

namespace bpp::detail {
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
	{ t.get_include_stack() } -> std::same_as<const std::vector<std::string>&>;
	{ t.get_source_file() } -> std::same_as<std::string>;
	{ t.get_lsp_mode() } -> std::same_as<bool>;
};

} // namespace bpp::detail
