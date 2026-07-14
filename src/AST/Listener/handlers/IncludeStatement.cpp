/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>
#include <AST/Parser.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

#include <filesystem>

namespace bpp::AST {

template <>
void Listener::enter(IncludeStatement* node) {
	if (!topmost_entity_is<bpp::IR::Program>()) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Include statements can only be used at the top level of a program");
	}
	auto current_program = std::static_pointer_cast<bpp::IR::Program>(entity_stack.top());

	const bool is_quoted_include = node->PATHTYPE() == AST::IncludeStatement::PathType::QUOTED;
	const bool is_dynamic_include = node->TYPE() == AST::IncludeStatement::IncludeType::DYNAMIC;
	const bool include_once = node->KEYWORD() == AST::IncludeStatement::IncludeKeyword::INCLUDE_ONCE;
	const auto& source_path_node = node->PATH();
	const auto& as_path_node = node->ASPATH();
	const std::string source_path = source_path_node.getValue().substr(1, source_path_node.getValue().length() - 2); // Remove surrounding quotes or angle brackets
	const std::string as_path = as_path_node.has_value()
		? as_path_node.value().getValue().substr(1, as_path_node.value().getValue().length() - 2)
		: ""; // Remove surrounding quotes

	std::filesystem::path include_path;

	if (!is_quoted_include) {
		// Search for the file in the include paths
		for (const auto& path : include_paths) {
			std::filesystem::path candidate_path = path / source_path;
			if (std::filesystem::exists(candidate_path)) {
				include_path = candidate_path;
				break;
			}
		}
	} else {
		// Quoted include:
		// Either a relative path (relative to the current source file) or an absolute path
		include_path = std::filesystem::path(source_path);
		if (!include_path.is_absolute()) {
			// Relative path: resolve it relative to the current source file's directory
			include_path = get_current_source_file().parent_path() / include_path;
		}
	}

	if (include_path.empty() || !std::filesystem::exists(include_path)) {
		throw bpp::ErrorHandling::SyntaxError(this, source_path_node, "File not found: '" + source_path + "'");
	}

	include_path = std::filesystem::canonical(include_path);

	auto res = included_files.insert(include_path);
	if (!res.second && include_once) return; // File has already been included and include_once is specified, so skip it

	if (include_chain.size() >= 200) { // TODO(@rail5): Can this limit be made configurable?
		throw bpp::ErrorHandling::SyntaxError(this, source_path_node, "Nested include depth exceeds maximum of 200");
	}

	include_chain.push_back(include_path);

	std::shared_ptr<bpp::AST::Program> included_program_ast_root;

	{
		AST::Parser parser;
		// FIXME(@rail5): utf16 mode for bpp-lsp
		//parser.setUTF16Mode(utf16_mode);
		parser.setIncludeChain(include_chain);

		// FIXME(@rail5): replacement file contents for bpp-lsp
		parser.setInputFromFilePath(include_path);

		included_program_ast_root = parser.program();
		for (const auto& e : parser.get_errors()) {
			e.print();
			this->program_has_errors = true;
		}
	}

	if (included_program_ast_root == nullptr) {
		include_chain.pop_back();
		throw bpp::ErrorHandling::SyntaxError(this, node, "Failed to parse included file: '" + include_path.string() + "'");
	}

	included_type_stack.push(is_dynamic_include ? IncludedType::DYNAMICALLY_INCLUDED : IncludedType::STATICALLY_INCLUDED);

	walk(included_program_ast_root.get());

	include_chain.pop_back();
	included_type_stack.pop();

	// If it's a dynamic include, add the `source` command
	if (is_dynamic_include) {
		// Determine the runtime path for the included file
		std::filesystem::path runtime_path;
		if (as_path_node.has_value()) { // The user provided an 'as' path (e.g., @include dynamic <Stack> as "/path/to/stack.sh"), just use that
			runtime_path = as_path;
		} else {
			runtime_path = include_path; // Original path, but replace the extension with .sh
			if (runtime_path.has_extension()) {
				runtime_path.replace_extension(".sh");
			} else {
				runtime_path += ".sh";
			}
		}

		auto runtime_source_command = std::make_shared<bpp::IR::CodeEntity>();
		runtime_source_command->add("if ! source \""
			+ runtime_path.string()
			+ "\"; then\n"
			"\t>&2 echo \"Bash++: Error: Failed to include file '" + runtime_path.string() + "'\"\n"
			"\texit 1\n"
			"fi\n"
		);
		current_program->add(runtime_source_command);
	}
}

template <>
void Listener::exit(IncludeStatement* /*node*/) {}

} // namespace bpp::AST
