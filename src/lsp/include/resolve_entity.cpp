/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "resolve_entity.h"
#include <unistd.h>

antlr4::tree::ParseTree* find_node_at_column(antlr4::tree::ParseTree* single_line_node, uint32_t column) {
	auto ctx = dynamic_cast<antlr4::ParserRuleContext*>(single_line_node);
	if (!ctx) {
		return nullptr;
	}

	auto* start = ctx->getStart();
	auto* stop = ctx->getStop();
	if (!start || !stop) {
		return nullptr;
	}

	int start_column = start->getCharPositionInLine();
	int stop_column = stop->getCharPositionInLine() + static_cast<int>(stop->getText().length());

	if (column < start_column || column > stop_column) {
		return nullptr; // Column is outside the range of this node
	}

	for (size_t i = 0; i < ctx->children.size(); i++) {
		auto* child = ctx->children[i];
		if (child) {
			auto* result = find_node_at_column(child, column);
			if (result) {
				return result; // Found a child node that matches the column
			}
		}
	}
	return ctx;
}

std::shared_ptr<bpp::bpp_entity> resolve_entity_at(
	const std::string& file,
	uint32_t line,
	uint32_t column,
	std::shared_ptr<bpp::bpp_program> program,
	const std::string& file_contents
) {
	if (program == nullptr || file.empty()) {
		return nullptr; // Invalid program or file
	}
	std::shared_ptr<bpp::bpp_entity> context = program->get_active_entity(file, line, column);
	if (context == nullptr) {
		context = program;
	}

	std::string line_content;
	// If we weren't provided with replacement contents,
	// Read from the file on the filesystem
	if (file_contents.empty()) {
		std::ifstream source_file(file);
		if (!source_file.is_open()) {
			return nullptr;
		}
		uint32_t current_line = 0;
		while (std::getline(source_file, line_content) && current_line < line) {
			current_line++;
		}
		source_file.close();

		if (current_line != line) {
			return nullptr; // Line does not exist
		}
	} else {
		// If replacement contents *were* provided,
		// Read from that instead
		std::istringstream stream(file_contents);
		uint32_t current_line = 0;
		while (std::getline(stream, line_content) && current_line < line) {
			current_line++;
		}

		if (current_line != line) {
			return nullptr; // Line does not exist
		}
	}

	// Use the ANTLR4 parser to parse the line content
	antlr4::ANTLRInputStream input(line_content);
	BashppLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	tokens.fill();

	BashppParser parser(&tokens);
	parser.removeErrorListeners();
	std::unique_ptr<antlr4::DiagnosticErrorListener> error_listener = std::make_unique<antlr4::DiagnosticErrorListener>();
	parser.addErrorListener(error_listener.get());
	parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());
	antlr4::tree::ParseTree* tree = parser.program();
	
	// Find the statement that intersects with the given column
	antlr4::tree::ParseTree* node = find_node_at_column(tree, column);
	if (!node) {
		return nullptr; // No node found at the specified column
	}

	// What type of statement is it?
	auto ctx = dynamic_cast<antlr4::ParserRuleContext*>(node);
	if (!ctx) {
		return nullptr; // Not a valid context
	}

	bool is_object_reference = false;
	std::deque<std::string> identifiers;

	// First, check if it's an object reference
	switch (ctx->getRuleIndex()) {
		case BashppParser::RuleObject_reference:
		{
			is_object_reference = true;
			auto object_ref_ctx = dynamic_cast<BashppParser::Object_referenceContext*>(ctx);
			if (!object_ref_ctx) {
				return nullptr; // Not a valid object reference context
			}

			for (const auto& id : object_ref_ctx->IDENTIFIER()) {
				if (id->getSymbol()->getCharPositionInLine() > column) {
					break;
				}
				identifiers.push_back(id->getText());
			}
		}
		break;
		case BashppParser::RuleObject_reference_as_lvalue:
		{
			is_object_reference = true;
			auto object_ref_lvalue_ctx = dynamic_cast<BashppParser::Object_reference_as_lvalueContext*>(ctx);
			if (!object_ref_lvalue_ctx) {
				return nullptr; // Not a valid object reference as lvalue context
			}

			identifiers.push_back(object_ref_lvalue_ctx->IDENTIFIER_LVALUE()->getText());

			for (const auto& id : object_ref_lvalue_ctx->IDENTIFIER()) {
				if (id->getSymbol()->getCharPositionInLine() > column) {
					break;
				}
				identifiers.push_back(id->getText());
			}
		}
		break;
		case BashppParser::RuleSelf_reference:
		{
			is_object_reference = true;
			auto self_ref_ctx = dynamic_cast<BashppParser::Self_referenceContext*>(ctx);
			if (!self_ref_ctx) {
				return nullptr; // Not a valid self reference context
			}

			if (self_ref_ctx->KEYWORD_THIS() != nullptr) {
				// This is a self-reference
				identifiers.push_back("this");
			} else if (self_ref_ctx->KEYWORD_SUPER() != nullptr) {
				// This is a super-reference
				identifiers.push_back("super");
			}

			for (const auto& id : self_ref_ctx->IDENTIFIER()) {
				if (id->getSymbol()->getCharPositionInLine() > column) {
					break;
				}
				identifiers.push_back(id->getText());
			}
		}
		break;
		case BashppParser::RuleSelf_reference_as_lvalue:
		{
			is_object_reference = true;
			auto self_ref_lvalue_ctx = dynamic_cast<BashppParser::Self_reference_as_lvalueContext*>(ctx);
			if (!self_ref_lvalue_ctx) {
				return nullptr; // Not a valid self reference as lvalue context
			}

			if (self_ref_lvalue_ctx->KEYWORD_THIS_LVALUE() != nullptr) {
				// This is a self-reference
				identifiers.push_back("this");
			} else if (self_ref_lvalue_ctx->KEYWORD_SUPER_LVALUE() != nullptr) {
				// This is a super-reference
				identifiers.push_back("super");
			}

			for (const auto& id : self_ref_lvalue_ctx->IDENTIFIER()) {
				if (id->getSymbol()->getCharPositionInLine() > column) {
					break;
				}
				identifiers.push_back(id->getText());
			}
		}
		break;
	}

	if (is_object_reference) {
		// Resolve the object reference
		bpp::entity_reference entity_ref = bpp::resolve_reference(
			file,
			context,
			&identifiers,
			program
		);
		return entity_ref.entity;
	}
	
	// If we're here, it's not an object reference. Keep checking
	switch (ctx->getRuleIndex()) {
		case BashppParser::RuleInclude_statement:
		{
			// Create a faux-entity that points to the included file line 0, column 0
			auto include_ctx = dynamic_cast<BashppParser::Include_statementContext*>(ctx);
			if (!include_ctx) {
				return nullptr; // Not a valid include statement context
			}
			bool local_include;
			antlr4::tree::TerminalNode* path_token;

			if (include_ctx->SYSTEM_INCLUDE_PATH() != nullptr) {
				local_include = false;
				path_token = include_ctx->SYSTEM_INCLUDE_PATH();
			} else {
				local_include = true;
				path_token = include_ctx->LOCAL_INCLUDE_PATH(0);
			}
			
			// Trim the first and last characters (either quote-marks or '<' and '>') from the path
			std::string source_filename = path_token->getText();
			source_filename = source_filename.substr(1, source_filename.length() - 2);

			// Resolve the path
			if (!local_include) {
				// Search for the file in the include paths
				bool found = false;
				for (const std::string& include_path : *program->get_include_paths()) {
					std::string full_path = include_path + "/" + source_filename;
					if (access(full_path.c_str(), F_OK) == 0) {
						source_filename = full_path;
						found = true;
						break;
					}
				}

				if (!found) {
					return nullptr; // File not found in include paths
				}
			} else {
				std::string current_directory;
				if (file == "<stdin>") {
					char current_working_directory[PATH_MAX];
					if (getcwd(current_working_directory, PATH_MAX) == nullptr) {
						throw internal_error("Could not get current working directory", ctx);
					}
					current_directory = current_working_directory;
				} else {
					current_directory = file.substr(0, file.find_last_of('/'));
				}

				// Append the source_filename to the directory
				source_filename = current_directory + "/" + source_filename;
			}

			// Get the full path of the file
			char full_path[PATH_MAX];
			if (realpath(source_filename.c_str(), full_path) == nullptr) {
				return nullptr; // File not found
			}

			std::shared_ptr<bpp::bpp_entity> faux_entity = std::make_shared<bpp::bpp_entity>();
			faux_entity->set_definition_position(source_filename, 0, 0);
			faux_entity->set_name(full_path);
			return faux_entity;
		}
		break;

		case BashppParser::RuleObject_instantiation:
		{
			// Resolve the class name being instantiated
			auto instantiation_ctx = dynamic_cast<BashppParser::Object_instantiationContext*>(ctx);
			if (!instantiation_ctx) {
				return nullptr; // Not a valid instantiation context
			}

			auto class_name_token = (instantiation_ctx->IDENTIFIER_LVALUE() != nullptr)
				? instantiation_ctx->IDENTIFIER_LVALUE()
				: instantiation_ctx->IDENTIFIER(0);
			auto class_pointer = context->get_class(class_name_token->getText());
			return class_pointer;
		}
		break;

		case BashppParser::RulePointer_declaration:
		{
			// Resolve the pointer type being declared
			auto pointer_ctx = dynamic_cast<BashppParser::Pointer_declarationContext*>(ctx);
			if (!pointer_ctx) {
				return nullptr; // Not a valid pointer declaration context
			}

			auto type_token = (pointer_ctx->IDENTIFIER_LVALUE() != nullptr)
				? pointer_ctx->IDENTIFIER_LVALUE()
				: pointer_ctx->IDENTIFIER(0);
			auto type_pointer = context->get_class(type_token->getText());
			return type_pointer;
		}
		break;

		case BashppParser::RuleParameter:
		{
			// Resolve the parameter class (if any)
			auto param_ctx = dynamic_cast<BashppParser::ParameterContext*>(ctx);
			if (!param_ctx) {
				return nullptr; // Not a valid parameter context
			}

			if (param_ctx->IDENTIFIER().size() > 1) {
				// There being more than one identifier means that this is a non-primitive parameter
				// Which therefore has a class type to resolve
				auto type_pointer = context->get_class(param_ctx->IDENTIFIER(0)->getText());
				return type_pointer;
			} else {
				return nullptr; // Primitive parameter, no class to resolve
			}
		}
		break;

		case BashppParser::RuleNew_statement:
		{
			// Resolve the class being instantiated with 'new'
			auto new_ctx = dynamic_cast<BashppParser::New_statementContext*>(ctx);
			if (!new_ctx) {
				return nullptr; // Not a valid new statement context
			}
			auto class_pointer = context->get_class(new_ctx->IDENTIFIER()->getText());
			return class_pointer;
		}
		break;

		case BashppParser::RuleDynamic_cast_statement:
		{
			// Resolve the class being cast to
			auto cast_ctx = dynamic_cast<BashppParser::Dynamic_cast_statementContext*>(ctx);
			if (!cast_ctx) {
				return nullptr; // Not a valid dynamic cast statement context
			}
			auto class_pointer = context->get_class(cast_ctx->IDENTIFIER()->getText());
			return class_pointer;
		}
		break;

		default:
			// For any other rule, we cannot resolve an entity
			return nullptr;
	}

	return nullptr;
}
