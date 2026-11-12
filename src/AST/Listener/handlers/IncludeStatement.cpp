/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>
#include <AST/Parser.h>
#include <IR/entities/Program.h>
#include <IR/entities/expressions/bpp/IncludeStatement.h>

#include <error/InternalError.h>

#include <filesystem>
#include <optional>

namespace bpp::AST {

namespace {

std::optional<std::filesystem::path> resolveAngleBracketPath(std::string_view file, const std::vector<std::filesystem::path>& include_paths) {
	for (const auto& p : include_paths) {
		std::filesystem::path candidate_path = p / file;
		if (std::filesystem::exists(candidate_path)) return std::filesystem::canonical(candidate_path);
	}
	return std::nullopt;
}

std::optional<std::filesystem::path> resolveQuotedPath(std::string_view file, const std::filesystem::path& current_directory) {
	std::filesystem::path result(file);
	if (!result.is_absolute()) result = current_directory / file;
	if (std::filesystem::exists(result)) return std::filesystem::canonical(result);
	return std::nullopt;
}

} // anonymous namespace

template <>
void Listener::enter(IncludeStatement* node) {
	if (!topmost_entity_is<bpp::IR::Program>() && !topmost_entity_is<bpp::IR::IncludeStatement>()) {
		error(node, "Include statements can only be used at the top level of a program");
	}

	const bool is_dynamic_include = node->TYPE() == AST::IncludeStatement::IncludeType::DYNAMIC;
	const bool include_always = node->KEYWORD() == AST::IncludeStatement::IncludeKeyword::INCLUDE_ALWAYS;
	const bool is_quoted_path = node->PATHTYPE() == AST::IncludeStatement::PathType::QUOTED;
	const std::string_view raw_path = node->PATH().getValue();
	const std::string_view specified_path = raw_path.substr(1, raw_path.size() - 2); // Remove the surrounding quotes or angle brackets

	const auto included_file =
		is_quoted_path
		? resolveQuotedPath(specified_path, getCurrentSourceFile().parent_path())
		: resolveAngleBracketPath(specified_path, include_paths);

	if (!included_file) error(node->PATH(), "File not found: " + std::string(raw_path));

	const bool already_included = included_files.contains(*included_file);
	const bool will_parse = !already_included || include_always;

	if (will_parse && include_chain.size() >= 200) { // TODO(@rail5): Can this limit be made configurable?
		error(node->PATH(), "Nested include depth exceeds maximum of 200");
	}

	auto include_entity = std::make_unique<bpp::IR::IncludeStatement>(*included_file, is_dynamic_include);
	include_entity->inherit(entity_stack.top());

	if (node->ASPATH().has_value()) {
		const std::string_view as_path = node->ASPATH()->getValue();
		include_entity->setAsPath(as_path.substr(1, as_path.size() - 2)); // Remove the surrounding quotes
	}

	if (source_index) {
		source_index->setDefinitionPosition(include_entity.get(), {*included_file, 0, 0, 0, 0});
		source_index->addReference(
			include_entity.get(),
			{getCurrentSourceFile(),
			node->getLine(), node->getColumn(),
			node->getEndPosition().line, node->getEndPosition().column}
		);
	}

	if (already_included && !include_always) {
		include_entity->disableCodegen();
		entity_stack.push(std::move(include_entity));
		include_chain.push_back(*included_file);
		return;
	}

	included_files.insert(*included_file);
	include_chain.push_back(*included_file);
	entity_stack.push(std::move(include_entity));

	std::unique_ptr<bpp::AST::Program> included_file_ast_root;

	{
		AST::Parser parser;
		parser.setUTF16Mode(utf16_mode);
		parser.setIncludeChain(include_chain);

		if (replacement_file_contents.contains(*included_file)) {
			parser.setInputFromStringContents(replacement_file_contents[*included_file], *included_file);
		} else {
			parser.setInputFromFilePath(*included_file);
		}

		parser.parse();
		included_file_ast_root = parser.release_program();
		for (const auto& e : parser.get_errors()) {
			if (source_index) source_index->addDiagnostic(*included_file, e);
			e.print_to_stderr();
			this->program_has_errors = true;
		}
	}

	if (!included_file_ast_root) {
		entity_stack.pop();
		include_chain.pop_back();
		error(node, "Failed to parse included file: '" + included_file->string() + "'");
	}

	node->addChildren(included_file_ast_root->releaseChildren()); // s.t. these will be walked by the listener implicitly
}

template <>
void Listener::exit(IncludeStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::IncludeStatement>(), "Topmost entity on stack is not an IncludeStatement");
	auto include_entity = entity_stack.pop_as<bpp::IR::IncludeStatement>();
	include_chain.pop_back();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity");
	auto* parent_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	parent_entity->adoptObjectsOf(include_entity.get());

	parent_entity->add(std::move(include_entity));
}

} // namespace bpp::AST
